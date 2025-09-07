/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "Checkpoint.h"
#include "ThreadManager.h"
#include "SaveStateManager.h"
#include "AltStack.h"
#include "MemArea.h"
#ifdef __unix__
#include "ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "MachVmMaps.h"
#endif
#include "StateHeader.h"
#include "ReservedMemory.h"
#include "SaveStateSaving.h"
#include "SaveStateLoading.h"
#include "FileDescriptorManip.h"

#include "TimeHolder.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#include "Utils.h"
#include "fileio/FileHandle.h"
#include "fileio/FileHandleList.h"
#include "fileio/SaveFile.h"
#include "fileio/SaveFileList.h"
#include "renderhud/RenderHUD.h"
#include "../shared/sockethelpers.h"
#ifdef __unix__
#include "../external/xcb/xcbint.h"
#include "xlib/xdisplay.h" // x11::gameDisplays
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <climits>
#include <stdint.h>
#include <sys/statvfs.h>
#include <cerrno>
#ifdef __unix__
#include <X11/Xlibint.h>
#include <X11/Xlib-xcb.h>
#endif
#ifdef __linux__
#include <sys/syscall.h>
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
/* Note that the ucontext passed to the signal handler is not exactly the same as the ucontext in <ucontext.h>
 * This is due to the ucontext in this instance being allocated kernel side
 * The libc may contain extensions to the ucontext, which the kernel won't add
 */
#include <ucontext.h>

#define ONE_MB 1024 * 1024

namespace libtas {

/* Savestate paths (for file storing)*/
static char pagemappath[1024] = "\0";
static char pagespath[1024] = "\0";

static char parentpagemappath[1024] = "\0";
static char parentpagespath[1024] = "\0";

static char basepagemappath[1024] = "\0";
static char basepagespath[1024] = "\0";

/* Savestate indexes (for RAM storing) */
static int ss_index = -1;
static int parent_ss_index = -1;
static int base_ss_index = -1;

/* Savestate ucontext (must be stored outside the alt stack) */
static ucontext_t ss_ucontext;
#if defined(__linux__) && (defined(__x86_64__) || defined(__i386__))
/* x86 ucontext keeps fp state outside of the actual ucontext, with it accessed by a pointer */
static _xstate ss_fpregset;
#ifndef UC_FP_XSTATE
#define UC_FP_XSTATE 1
#endif
#endif

static void readAllAreas();
static int reallocateArea(Area *saved_area, Area *current_area);
static void readAnArea(SaveStateLoading &saved_area, int spmfd, SaveStateLoading &parent_state, SaveStateLoading &base_state);
static void syncFileDescriptors();
static void readASavefile(SaveStateLoading &saved_state);

static void writeAllAreas(bool base);
static size_t writeAnArea(SaveStateSaving &state, Area &area, int spmfd, SaveStateLoading &parent_state, SaveStateLoading &base_state, bool base);
static size_t writeSaveFiles(SaveStateSaving &state);

void Checkpoint::setSavestatePath(std::string path)
{
    std::string pmpath = path + ".pm";
    strncpy(pagemappath, pmpath.c_str(), 1023);

    std::string ppath = path + ".p";
    strncpy(pagespath, ppath.c_str(), 1023);
}

void Checkpoint::setBaseSavestatePath(std::string path)
{
    std::string pmpath = path + ".pm";
    strncpy(basepagemappath, pmpath.c_str(), 1023);

    std::string ppath = path + ".p";
    strncpy(basepagespath, ppath.c_str(), 1023);
}

void Checkpoint::setSavestateIndex(int index)
{
    ss_index = index;
}

void Checkpoint::setBaseSavestateIndex(int index)
{
    base_ss_index = index;
}

void Checkpoint::setCurrentToParent()
{
    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        strcpy(parentpagemappath, pagemappath);
        strcpy(parentpagespath, pagespath);
        parent_ss_index = ss_index;
    }
}

static void resetParent()
{
    parentpagemappath[0] = '\0';
    parentpagespath[0] = '\0';
    parent_ss_index = -1;
}

int Checkpoint::checkCheckpoint()
{
    /* TODO: Find another way to check for space, because mapped memory is
     * way bigger than final savestate size. */
    return SaveStateManager::ESTATE_OK;

    /* Get an estimation of the savestate space */
    uint64_t savestate_size = 0;

#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Read the first current area */
    Area area;
    bool not_eof = memMapLayout.getNextArea(&area);

    while (not_eof) {
        if (!area.skip) {
            savestate_size += area.size;
        }
        not_eof = memMapLayout.getNextArea(&area);
    }

    /* Get the savestate directory */
    std::string savestate_str = pagemappath;
    size_t sep = savestate_str.find_last_of("/");
    if (sep != std::string::npos)
        savestate_str.resize(sep);

    struct statvfs devData;
    int ret;
    if ((ret = statvfs(savestate_str.c_str(), &devData)) >= 0) {
        uint64_t available_size = static_cast<uint64_t>(devData.f_bavail) * devData.f_bsize;
        if (savestate_size > available_size) {
            return SaveStateManager::ESTATE_NOMEM;
        }
    }

    return SaveStateManager::ESTATE_OK;
}

int Checkpoint::checkRestore()
{
    /* Check that the savestate files exist */
    struct stat sb;
    if (stat(pagemappath, &sb) == -1) {
        return SaveStateManager::ESTATE_NOSTATE;
    }
    if (stat(pagespath, &sb) == -1) {
        return SaveStateManager::ESTATE_NOSTATE;
    }

    int pmfd = open(pagemappath, O_RDONLY);
    if (pmfd == -1)
        return SaveStateManager::ESTATE_NOSTATE;

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));
    close(pmfd);

    return SaveStateManager::ESTATE_OK;
}

void Checkpoint::handler(int signum, siginfo_t *info, void *ucontext)
{
    GlobalNative gn;

#ifdef __unix__
    /* Check that we are using our alternate stack by looking at the address
     * of this local variable.
     */
    Display *display = x11::gameDisplays[0];
    if ((&display < ReservedMemory::getAddr(0)) ||
        (&display >= ReservedMemory::getAddr(ReservedMemory::getSize()))) {
        LOG(LL_FATAL, LCF_CHECKPOINT, "Checkpoint code is not running on alternate stack");
        return;
    }
#endif

    if (SaveStateManager::isLoading()) {
#ifdef __unix__
        /* Before reading from the savestate, we must keep some values from
         * the connection to the X server, because they are used for checking
         * consistency of requests. We can store them in this (alternate) stack
         * which will be preserved.
         */
        uint64_t last_request_read[GAMEDISPLAYNUM];
        uint64_t request[GAMEDISPLAYNUM];
        xcb_connection_t xcb_conn[GAMEDISPLAYNUM];

        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (x11::gameDisplays[i]) {
#ifdef X_DPY_GET_LAST_REQUEST_READ
                last_request_read[i] = X_DPY_GET_LAST_REQUEST_READ(x11::gameDisplays[i]);
                request[i] = X_DPY_GET_REQUEST(x11::gameDisplays[i]);
#else
                last_request_read[i] = static_cast<uint64_t>(x11::gameDisplays[i]->last_request_read);
                request[i] = static_cast<uint64_t>(x11::gameDisplays[i]->request);
#endif

                /* Copy the entire xcb connection struct */
                xcb_connection_t *cur_xcb_conn = XGetXCBConnection(x11::gameDisplays[i]);
                memcpy(&xcb_conn[i], cur_xcb_conn, sizeof(xcb_connection_t));
            }
        }
#endif

        TimeHolder old_time, new_time, delta_time;
        clock_gettime(CLOCK_MONOTONIC, &old_time);
        readAllAreas();
        clock_gettime(CLOCK_MONOTONIC, &new_time);
        delta_time = new_time - old_time;
        LOG(LL_INFO, LCF_CHECKPOINT, "Loaded state %d in %f seconds", ss_index, delta_time.tv_sec + ((double)delta_time.tv_nsec) / 1000000000.0);

        /* Loading state was overwritten, putting the right value again */
        SaveStateManager::setLoading();

#ifdef __unix__
        /* Restoring the display values */
        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (x11::gameDisplays[i]) {
#ifdef X_DPY_SET_LAST_REQUEST_READ
                X_DPY_SET_LAST_REQUEST_READ(x11::gameDisplays[i], last_request_read[i]);
                X_DPY_SET_REQUEST(x11::gameDisplays[i], request[i]);
#else
                gameDisplays[i]->last_request_read = static_cast<unsigned long>(last_request_read[i]);
                gameDisplays[i]->request = static_cast<unsigned long>(request[i]);
#endif

                /* Restore the entire xcb connection struct */
                xcb_connection_t *cur_xcb_conn = XGetXCBConnection(x11::gameDisplays[i]);
                memcpy(cur_xcb_conn, &xcb_conn[i], sizeof(xcb_connection_t));
            }
        }
#endif

        /* We must restore the ucontext from the savestate */
#if defined(__linux__) && (defined(__x86_64__) || defined(__i386__))
        /* Make sure we properly restore the fp state */
        ss_ucontext.uc_mcontext.fpregs = static_cast<ucontext_t*>(ucontext)->uc_mcontext.fpregs;
        if (ss_ucontext.uc_flags & UC_FP_XSTATE) {
            memcpy(reinterpret_cast<_xstate*>(ss_ucontext.uc_mcontext.fpregs), &ss_fpregset, sizeof(_xstate));
        }
        else {
            memcpy(reinterpret_cast<_fpstate*>(ss_ucontext.uc_mcontext.fpregs), &ss_fpregset.fpstate, sizeof(_fpstate));
        }
        memcpy(ucontext, &ss_ucontext, offsetof(ucontext_t, __fpregs_mem));
#else
        memcpy(ucontext, &ss_ucontext, sizeof(ucontext_t));
#endif
    }
    else {
        /* Check that base savestate exists, otherwise save it */
        if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
            struct stat sb;
            if (stat(basepagemappath, &sb) == -1) {
                resetParent();
                writeAllAreas(true);
            }
        }
        /* We must store the passed ucontext in the savestate */
#if defined(__linux__) && (defined(__x86_64__) || defined(__i386__))
        memcpy(&ss_ucontext, ucontext, offsetof(ucontext_t, __fpregs_mem));
        /* Make sure we properly save the fp state */
        if (ss_ucontext.uc_flags & UC_FP_XSTATE) {
            memcpy(&ss_fpregset, reinterpret_cast<_xstate*>(ss_ucontext.uc_mcontext.fpregs), sizeof(_xstate));
        }
        else {
            memcpy(&ss_fpregset.fpstate, reinterpret_cast<_fpstate*>(ss_ucontext.uc_mcontext.fpregs), sizeof(_fpstate));
        }
#else
        memcpy(&ss_ucontext, ucontext, sizeof(ucontext_t));
#endif

        writeAllAreas(false);
    }
}

void Checkpoint::getStateHeader(StateHeader* sh)
{
    /* Thanks to checkRestore() being called before this, savestate is garanteed 
     * to be present */
    SaveStateLoading saved_state(pagemappath, pagespath);
    saved_state.readHeader(sh);
}

static void readAllAreas()
{
    /* Before opening any file, we must manipulate returned file descriptors, so
     * that they don't collide with the file descriptors that we may want to 
     * recover after loading the state. After the following call, all returned
     * file descriptors will be above a certain high value. */
    FileDescriptorManip::reserveUntilState();
    
    SaveStateLoading saved_state(pagemappath, pagespath);

    int spmfd = open("/proc/self/pagemap", O_RDONLY);
    MYASSERT(spmfd != -1);

    int crfd = -1;
    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        crfd = open("/proc/self/clear_refs", O_WRONLY);
        MYASSERT(crfd != -1);
    }

    /* Read the savestate header */
    StateHeader sh;
    saved_state.readHeader(&sh);

    Area current_area;
    Area& saved_area = saved_state.getArea();

    LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Load base and parent savestates */
    SaveStateLoading parent_state(parentpagemappath, parentpagespath);
    SaveStateLoading base_state(basepagemappath, basepagespath);

    /* Now that we have opened all files we need, and *before* doing the actual
     * state loading, we can clear our file descriptor reserve. If doing this
     * after state loading, the variables used for keeping track of fds would
     * have been overwritten... */
    FileDescriptorManip::closeAll();

    /* Read the first current area */
    bool not_eof = memMapLayout.getNextArea(&current_area);

    /* Reallocate areas to match the savestate areas. Nothing is written yet */
    while (saved_area.isStandard() || not_eof) {

        /* Check for matching areas */
        int cmp = reallocateArea(&saved_area, &current_area);
        if (cmp == 0) {
            /* Areas matched, we advance both areas */
            saved_area = saved_state.nextArea();
            not_eof = memMapLayout.getNextArea(&current_area);
        }
        if (cmp > 0) {
            /* Saved area is greater, advance current area */
            not_eof = memMapLayout.getNextArea(&current_area);
        }
        if (cmp < 0) {
            /* Saved area is smaller, advance saved area */
            saved_area = saved_state.nextArea();
        }
    }
    
    /* Now that the memory layout matches the savestate, we load savestate into memory */
    saved_state.restart();
    saved_area = saved_state.getArea();
    
    /* If the loading savestate and the parent savestate are the same, pass the
    * same SaveStateLoading object to readAnArea because two SaveStateLoading objects
    * handling the same file descriptor will mess up the file offset. */
    bool same_state = (ss_index == parent_ss_index);
    while (saved_area.isStandard()) {
        readAnArea(saved_state, spmfd, same_state?saved_state:parent_state, base_state);
        saved_area = saved_state.nextArea();
    }
    
    /* Before restoring savefiles, we open and close file descriptors to be in 
     * sync with when the savestate was made. */
    syncFileDescriptors();
    
    /* The remaining areas are savefiles */
    while (saved_area) {
        readASavefile(saved_state);
        saved_area = saved_state.nextArea();
    }

    if (crfd != -1) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
        close(crfd);
    }

    close(spmfd);
}

static int reallocateArea(Area *saved_area, Area *current_area)
{
    /* Do Areas start on the same address? */
    if ((saved_area->isStandard()) && (current_area->addr != nullptr) &&
        (saved_area->addr == current_area->addr)) {

        /* If it is not the same area, unmap the current area */
        if ((strcmp(saved_area->name, current_area->name) != 0) ||
            (saved_area->flags != current_area->flags)) {

            LOG(LL_DEBUG, LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
            MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            return 1;
        }

        size_t copy_size = (saved_area->size<current_area->size)?saved_area->size:current_area->size;

        if (saved_area->size != current_area->size) {

            /* Special case for stacks, always try to resize the Area */
            if (saved_area->flags & Area::AREA_STACK) {
#ifdef __linux__
                LOG(LL_DEBUG, LCF_CHECKPOINT, "Changing stack size from %d to %d", current_area->size, saved_area->size);
                void *newAddr = mremap(current_area->addr, current_area->size, saved_area->size, 0);

                if (newAddr == MAP_FAILED) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "Resizing failed");
                    return 0;
                }

                if (newAddr != saved_area->addr) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "mremap relocated the area");
                    return 0;
                }
#else
                LOG(LL_FATAL, LCF_CHECKPOINT, "stack size changed but mremap is not available");
#endif
                copy_size = saved_area->size;
            }

            /* Special case for heap, use brk instead */
            if (saved_area->flags & Area::AREA_HEAP) {
                LOG(LL_DEBUG, LCF_CHECKPOINT, "Changing heap size from %d to %d", current_area->size, saved_area->size);

#ifdef __linux__
                int ret = brk(saved_area->endAddr);
#else
                intptr_t ret = reinterpret_cast<intptr_t>(brk(saved_area->endAddr));
#endif

                if (ret < 0) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "brk failed");
                    return 0;
                }

                copy_size = saved_area->size;
            }
        }

        /* Apply the protections from the saved area if needed */
        if (saved_area->prot != current_area->prot) {
            MYASSERT(mprotect(saved_area->addr, copy_size, saved_area->prot) == 0)
        }

        if ((saved_area->endAddr == current_area->endAddr) ||
            (saved_area->name[0] == '[')) {
            /* If the Areas have the same size, or it was a special section,
             * we have nothing to do.
             */
            return 0;
        }

        if (saved_area->endAddr > current_area->endAddr) {
            /* If there is still some memory in the savestate to write,
             * we update the area
             */
            saved_area->addr = current_area->endAddr;
            saved_area->size -= copy_size;
            return 1;
        }

        if (saved_area->endAddr < current_area->endAddr) {
            /* There is extra memory that we have to deal with */
            current_area->addr = saved_area->endAddr;
            current_area->size -= copy_size;
            return -1;
        }
    }

    if ((!saved_area->isStandard()) || (saved_area->addr > current_area->addr)) {
        /* Our current area starts before the saved area */
        LOG(LL_DEBUG, LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
        MYASSERT(munmap(current_area->addr, current_area->size) == 0)

        return 1;
    }

    if ((current_area->addr == nullptr) || (saved_area->addr < current_area->addr)) {

        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* Areas are overlapping, we unmap the current area until there is no more overlapping */
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
            MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            return 1;
        }

        /* This saved area must be allocated */
        LOG(LL_DEBUG, LCF_CHECKPOINT, "Region %p (%s) with size %d must be allocated", saved_area->addr, saved_area->name, saved_area->size);

        /* For file mapping, we try to use an already existing file descriptor
         * of that file. This is even necessary when the file was deleted.
         * If not file descriptor, we try to open the file. */
        int imagefd = saved_area->fd;
        if ((saved_area->flags & Area::AREA_FILE) && (imagefd == -1)) {
            /* We shouldn't be creating any special section such as [heap] or [stack] */
            MYASSERT(saved_area->name[0] == '/')

            imagefd = open(saved_area->name, O_RDONLY);
            if (imagefd >= 0) {
                /* We check if the current file has a size that can be mapped 
                 * to the saved memory region. */
                off_t curr_size = lseek(imagefd, 0, SEEK_END);

                if (curr_size == -1) {
                    LOG(LL_WARN, LCF_CHECKPOINT, "Could not seek to end of file %s", saved_area->name);
                }
                else {
                    /* Round to upper page size */
                    size_t aligned_curr_size = static_cast<size_t>(((curr_size + 4095) / 4096) * 4096);
                    
                    if (aligned_curr_size < (saved_area->offset + saved_area->size)) {
                        LOG(LL_WARN, LCF_CHECKPOINT, "File %s was mapped to offset %zu but has a current size of %zd", saved_area->name, saved_area->offset + saved_area->size, curr_size);
                    }
                }
            }
            else {
                /* We could not open the file, map the section as anonymous */
                saved_area->flags &= ~Area::AREA_FILE;
                saved_area->flags |= Area::AREA_ANON;
                saved_area->offset = 0;
            }
        }

        if (saved_area->flags & Area::AREA_ANON) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Restoring anonymous area, %d bytes at %p", saved_area->size, saved_area->addr);
        } else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Restoring non-anonymous area, %d bytes at %p from %s + %d", saved_area->size, saved_area->addr, saved_area->name, saved_area->offset);
        }

        /* Create the memory area */
        void *mmappedat = mmap(saved_area->addr, saved_area->size, saved_area->prot,
                               saved_area->toMmapFlag(), imagefd, saved_area->offset);

        if (mmappedat == MAP_FAILED) {
            LOG(LL_FATAL, LCF_CHECKPOINT, "Mapping %d bytes at %p failed: errno %d", saved_area->size, saved_area->addr, errno);
        }

        if (mmappedat != saved_area->addr) {
            LOG(LL_FATAL, LCF_CHECKPOINT, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
        }

        /* Close image file if we opened it */
        if ((imagefd != -1) && (saved_area->fd != imagefd)) {
            close(imagefd);
        }

        return -1;
    }

    MYASSERT(false)
    return 0;
}

static void readAnArea(SaveStateLoading &saved_state, int spmfd, SaveStateLoading &parent_state, SaveStateLoading &base_state)
{
    const Area& saved_area = saved_state.getArea();

    if (saved_area.skip)
        return;

    /* Skip if both saved and current area are uncommitted */
    if (saved_area.uncommitted && saved_area.isUncommitted(spmfd))
        return;

    saved_area.print("Restore");

    /* Add read/write permission to the area.
     * Because adding write permission increases the commit charge, it can fail
     * on very large uncommitted memory (Celeste64 -> 274GB memory segment).
     * So, I will call mprotect() on individual memory pages when needed */
    if (!(saved_area.prot & PROT_READ)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot | PROT_READ) == 0)
    }

    MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(saved_area.addr) / (4096/8)), SEEK_SET));

    /* Number of pages in the area */
    size_t nb_pages = saved_area.size / 4096;

    /* Index of the current area page */
    size_t page_i = 0;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    /* Stats to print */
    int pagecount_zero_or_file = 0;
    int pagecount_full = 0;
    int pagecount_base = 0;
    int pagecount_skip = 0;

    /* Original file descriptor. Do not open the file yet, because we may not 
     * need to open it at all. */
    int orig_fd = -1;

    /* File descriptor of the shared mapped file, to check for holes */
    int shared_fd = -1;
    off_t shared_next_off_hole = -1;
    off_t shared_next_off_data = -1;

    if (saved_area.flags & Area::AREA_SHARED) {
        /* Try first to open the file */
        shared_fd = open(saved_area.name, O_RDONLY);

        /* If no file, shared mappings always have an underlying file accessible
         * inside /proc/self/map_files/ */
        if (shared_fd == -1) {
            shared_fd = open(saved_area.map_file, O_RDONLY);
        }

        if (shared_fd != -1) {
            /* Try to seek here */
            shared_next_off_hole = lseek(shared_fd, saved_area.offset, SEEK_HOLE);                
            if (shared_next_off_hole == -1) {
                LOG(LL_DEBUG, LCF_CHECKPOINT, "     Could not seek into shared map file %s", saved_area.map_file);
                close(shared_fd);
                shared_fd = -1;
            }
            else {
                shared_next_off_data = lseek(shared_fd, saved_area.offset, SEEK_DATA);
                if (shared_next_off_data == -1) shared_next_off_data = LONG_MAX; // No data, set to past end of file
                shared_next_off_data &= ~0xfffl; // truncate to page size, this may not be necessary
                shared_next_off_hole &= ~0xfffl;
            }
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "     Could not open shared map file %s", saved_area.map_file);
        }
    }

    char* endAddr = static_cast<char*>(saved_area.endAddr);
    for (char* curAddr = static_cast<char*>(saved_area.addr);
    curAddr < endAddr;
    curAddr += 4096, page_i++, pagemap_i++) {

        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if (pagemap_i >= 512) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        char flag = saved_area.uncommitted ? Area::NO_PAGE : saved_state.getNextPageFlag();

        /* Gather the flag for the page map */
        uint64_t page = pagemaps[pagemap_i++];
        bool soft_dirty = page & (0x1ull << 55);
        bool page_file = page & (0x1ull << 61);
        bool page_present = page & (0x1ull << 63);

        /* It seems that static memory is both zero and unmapped, so we still
         * need to memset the region if it was mapped.
         *
         * TODO: This setting breaks savestates in Freedom Planet, when saving
         * on one stage, advancing to the next stage, loading the state and 
         * advancing to next stage again. */
        if (flag == Area::NO_PAGE) {
            if (saved_area.flags & Area::AREA_PRIV) {
                if (page_present && (!Utils::isZeroPage(static_cast<void*>(curAddr)))) {
                    if (!(saved_area.prot & PROT_WRITE)) {
                        MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                    }
                    memset(static_cast<void*>(curAddr), 0, 4096);
                    pagecount_zero_or_file++;
                }
                else {
                    pagecount_skip++;
                }
            }
            else {
                /* Check for a hole in the shared mapped file (works even for anonymous
                 * mappings, which still have an underlying file) */
                if (shared_fd != -1) {
                    off_t current_off = saved_area.offset + page_i*4096;
                    
                    if ((current_off > shared_next_off_data) && (current_off > shared_next_off_hole)) {
                        /* data and hole offsets are obsolete, update both of them */
                        shared_next_off_data = lseek(shared_fd, current_off, SEEK_DATA);
                        if (shared_next_off_data == -1) shared_next_off_data = LONG_MAX; // No data, set to past end of file
                        shared_next_off_data &= ~0xfffl;
                        shared_next_off_hole = lseek(shared_fd, current_off, SEEK_HOLE);
                        shared_next_off_hole &= ~0xfffl;
                    }
                    
                    if ((current_off == shared_next_off_data) || (current_off < shared_next_off_hole)) {
                        /* Current page is data, but saved page is a hole, so
                         * we memset the page. TODO: use fallocate to recreate
                         * the hole, but that would need opening the file in 
                         * write mode. */

                        if (!(saved_area.prot & PROT_WRITE)) {
                            MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                        }
                        memset(static_cast<void*>(curAddr), 0, 4096);
                        pagecount_zero_or_file++;
                    }
                    else if ((current_off == shared_next_off_hole) || (current_off < shared_next_off_data)) {
                        /* Current page is hole like saved page is, we have
                         * nothing to do. */
                        pagecount_skip++;
                    }
                    else {
                        LOG(LL_WARN, LCF_CHECKPOINT, "     Seeking into shared file shows neither data or hole!");
                        LOG(LL_WARN, LCF_CHECKPOINT, "     Start offset %lx, end of file %lx, current offset %lx, current data offset %lx, current hole offset %lx", 
                            saved_area.offset, saved_area.offset + saved_area.size, current_off, shared_next_off_data, shared_next_off_hole);
                    }
                }
                else {
                    LOG(LL_WARN, LCF_CHECKPOINT, "     Shared map file %s was not found even if it existed when the savestate was made!", curAddr);
                }
            }
            continue;
        }
        
        if (flag == Area::ZERO_PAGE) {
            if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
                /* In case incremental savestates is enabled, we can guess that
                 * the page is already zero if the parent page is zero and the
                 * page was not modified since. In that case, we can skip the memset. */
                if (soft_dirty ||
                    parent_state.getPageFlag(curAddr) != Area::ZERO_PAGE) {
                    if (!(saved_area.prot & PROT_WRITE)) {
                        MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                    }
                    memset(static_cast<void*>(curAddr), 0, 4096);
                    pagecount_zero_or_file++;
                }
                else {
                    pagecount_skip++;
                    if (Global::shared_config.logging_level >= LL_DEBUG) {
                        if (!Utils::isZeroPage(static_cast<void*>(curAddr))) {
                            LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be zero, but is actually not!", curAddr);
                        }
                    }
                }
            }
            else {
                /* Only memset if the page is not zero, to prevent an actual
                 * allocation if the page was allocated but never used. */
                if (!Utils::isZeroPage(static_cast<void*>(curAddr))) {
                    if (!(saved_area.prot & PROT_WRITE)) {
                        MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                    }
                    memset(static_cast<void*>(curAddr), 0, 4096);
                    pagecount_zero_or_file++;
                }
                else {
                    pagecount_skip++;                    
                }
            }
            continue;
        }
        
        if (flag == Area::FILE_PAGE) {
            if (page_present && !page_file) {
                /* Page was modified, we need to restore the content of the
                 * original file */
                if (orig_fd == -1) {
                    /* File was not opened yet, do it */
                    orig_fd = open(saved_area.name, O_RDONLY);
                    if (orig_fd == -1) {
                        LOG(LL_WARN, LCF_CHECKPOINT, "     File %s was opened to recover one page of the file mapped region, and failed!", saved_area.name);                                
                        orig_fd = -2; // Save a special value, so we don't try again
                    }
                }
                
                if (orig_fd >= 0) {
                    if (!(saved_area.prot & PROT_WRITE)) {
                        MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                    }
                    
                    lseek(orig_fd, page_i*4096 + saved_area.offset, SEEK_SET);
                    Utils::readAll(orig_fd, curAddr, 4096);
                    pagecount_zero_or_file++;
                }
            }
            else {
                pagecount_skip++;
            }
            continue;
        }

        /* From here, we are sure to write to the page */
        if (!(saved_area.prot & PROT_WRITE)) {
            MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
        }
        
        if (flag == Area::BASE_PAGE) {
            /* The memory page of the loading savestate is the same as the base
             * savestate. We must check if we actually need to read from the
             * base savestate or if the page already contains the correct values.
             */

            char parent_flag = parent_state.getPageFlag(curAddr);

            if (parent_flag != Area::BASE_PAGE) {
                /* Memory page has been modified between the two savestates.
                 * We must read from the base savestate.
                 */
                base_state.getPageFlag(curAddr);
                base_state.queuePageLoad(curAddr);
                pagecount_base++;
                continue;
            }
            
            if (soft_dirty) {
                /* Memory page has been modified after parent state.
                 * We must read from the base savestate.
                 */
                base_state.getPageFlag(curAddr);
                base_state.queuePageLoad(curAddr);
                pagecount_base++;
                continue;
            }

            pagecount_skip++;
            /* Double-check that page is indeed identical to the base savestate */
            if (Global::shared_config.logging_level >= LL_DEBUG) {
                base_state.getPageFlag(curAddr);
                if (!base_state.debugIsMatchingPage(curAddr)) {
                    LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be identical to base state, but is actually not!", curAddr);                                
                }
            }
        }
        else {
            pagecount_full++;
            saved_state.queuePageLoad(curAddr);
        }
    }
    base_state.finishLoad();
    saved_state.finishLoad();

    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d, base: %d, skipped: %d", pagecount_full, pagecount_zero_or_file, pagecount_base, pagecount_skip);
    }
    else {
        LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d, skipped: %d", pagecount_full, pagecount_zero_or_file, pagecount_skip);
    }

    saved_state.checkHash();

    /* Recover permission to the area */
    if (!(saved_area.prot & PROT_WRITE) || !(saved_area.prot & PROT_READ)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
    }
    
    if (orig_fd >= 0)
        close(orig_fd);

    if (shared_fd >= 0)
        close(shared_fd);
}

/* Recreate and close all file descriptors, so that the current list match the
 * list that was present when the state was saved. */
static void syncFileDescriptors()
{
    /* Browse all possible values of fd. We don't go past our reserve value,
     * which contains the fds for the state loading procedure */ 
    for (int fd = 3; fd < FileDescriptorManip::reserveState(); fd++) {
        /* Query for a registered file handle (excluding pipes) */
        const FileHandle& fh = FileHandleList::fileHandleFromFd(fd);
        bool fdRegistered = (fh.fds[0] == fd) && !fh.isPipe();

        /* Look at existing file descriptor and get symlink */
        char fd_str[25];
        sprintf(fd_str, "/proc/self/fd/%d", fd);

        char buf[1024] = {};
        ssize_t buf_size = readlink(fd_str, buf, 1024);
        bool fdOpened = false;

        if (buf_size != -1) {
            if (buf_size == 1024) {
                /* Truncation occured */
                buf[1023] = '\0';
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Symlink of file fd %d was truncated: %s", fd, buf);
            }

            /* Don't add special files, such as sockets or pipes */
            if ((buf[0] == '/') && (0 != strncmp(buf, "/dev/", 5))) {
                fdOpened = true;
            }
        }

        /* Handle all cases of fd */
        if (!fdRegistered && !fdOpened) {
            continue;
        }
        if (fdRegistered && fdOpened) {
            /* Check for matching path */
            if (0 != strcmp(fh.fileName(), buf)) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "File descriptor %d is currently linked to %s, but was linked to %s in savestate", fd, buf, fh.fileName());
            }
        }
        if (!fdRegistered && fdOpened) {
            /* The file descriptor must be closed, because it is not present in
             * the savestate */
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "File %s with fd %d should not be present after loading the state, so it is closed", buf, fd);
            close(fd);
        }
        if (fdRegistered && !fdOpened) {
            /* The file descriptor must be opened, because it is present in
             * the savestate */
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "File %s with fd %d should be opened", fh.fileName(), fd);

            /* We must open the file with the same fd as the one saved in the state */
            int next_fd = FileDescriptorManip::enforceNext(fd);
            
            if (next_fd == -1) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because fd manipulation failed", fd);
                continue;
            }

            if (next_fd > fd) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because we went past it after calling dup(), which returned %d", fd, next_fd);
                continue;
            }

            if (next_fd != fd) {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Could not recreate fd %d because it is already opened", fd);
                continue;
            }

            /* Recreate a memfd if savefile */
            const SaveFile* sf = SaveFileList::getSaveFile(fd);
            if (sf) {
                int new_fd = syscall(SYS_memfd_create, sf->filename.c_str(), 0);
                if (new_fd < 0) {
                    LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "Could not create memfd");
                    continue;                    
                }
                if (new_fd != fd) {
                    LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Recreate fd was supposed to be %d but is instead %d...", fd, new_fd);
                    close(new_fd);
                    continue;
                }
            }
            else {
                /* TODO: We assume that the non-savefile file is read-only for now */
                int new_fd = open(fh.fileName(), O_RDONLY);
                if (new_fd < 0) {
                    LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "Could not open file %s", fh.fileName());
                    continue;                    
                }
                if (new_fd != fd) {
                    LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "Recreate fd was supposed to be %d but is instead %d...", fd, new_fd);
                    close(new_fd);
                    continue;
                }
            }
        }
    }

    FileDescriptorManip::closeAll();
}


/* Write a memory area into the savestate. Returns the size of the area in bytes */
static void readASavefile(SaveStateLoading &saved_state)
{
    const Area& saved_area = saved_state.getArea();

    if (!(saved_area.flags & Area::AREA_SAVEFILE))
        return;
    
    int ret = ftruncate(saved_area.fd, saved_area.size);
    
    if (ret < 0) {
        LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not truncate the savefile %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);
        return;
    }
    
    void* saved_area_addr = mmap(nullptr, saved_area.size, PROT_WRITE, MAP_SHARED, saved_area.fd, 0);
        
    if (saved_area_addr == MAP_FAILED) {
        LOG(LL_ERROR, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not map file %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);
        return;
    }

    LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "Restore savefile %s with fd %d and size %zu", saved_area.name, saved_area.fd, saved_area.size);

    /* Map the original file */
    int orig_fd = -1;
    void* orig_file_mapped_addr = nullptr;
    
    struct stat filestat;
    int rv = stat(saved_area.name, &filestat);

    if (rv < 0)
        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     No original savefile to use");
    else if (! S_ISREG(filestat.st_mode) || (filestat.st_size == 0))
        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "    Original file is missing or not regular, skip it");
    else {
        orig_fd = open(saved_area.name, O_RDONLY);
        
        if (orig_fd == -1)
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not locate original savefile");
        else {
            orig_file_mapped_addr = mmap(nullptr, filestat.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_FILE, orig_fd, 0);
            close(orig_fd);
        }
    }
    
    /* Go through both original and current mapped files, and only store 
     * pages that are different */
    char* mapped_addr_begin = static_cast<char*>(saved_area_addr);
    char* mapped_addr_end = mapped_addr_begin + saved_area.size;
    
    char* orig_file_mapped_begin = static_cast<char*>(orig_file_mapped_addr);

    for (;mapped_addr_begin < mapped_addr_end; mapped_addr_begin += 4096, orig_file_mapped_begin += 4096) {
        size_t page_len = (mapped_addr_end - mapped_addr_begin) > 4096 ? 4096 : (mapped_addr_end - mapped_addr_begin);

        char flag = saved_state.getNextPageFlag();

        if (flag == Area::FILE_PAGE) {
            /* Copy the original file into this file */
            if (orig_file_mapped_addr) {
                memcpy(mapped_addr_begin, orig_file_mapped_begin, page_len);                
            }
            else {
                LOG(LL_WARN, LCF_CHECKPOINT | LCF_FILEIO, "     Page %p is not stored but original file is missing!", mapped_addr_begin);
            }
        }
        else {
            saved_state.queuePageLoad(mapped_addr_begin);
        }
    }
    
    saved_state.finishLoad();
    
    saved_state.getArea().addr = saved_area_addr;
    saved_state.checkHash();

    /* Unmap files */
    if (saved_area_addr != MAP_FAILED)
        munmap(saved_area_addr, saved_area.size);

    if (orig_file_mapped_addr != MAP_FAILED)
        munmap(orig_file_mapped_addr, filestat.st_size);
}

static void writeAllAreas(bool base)
{
    if (Global::shared_config.savestate_settings & SharedConfig::SS_FORK) {
        pid_t pid;
        pid = fork();
        if (pid != 0)
            return;

        ThreadManager::restoreTid();
    }

    TimeHolder old_time, new_time, delta_time;
    clock_gettime(CLOCK_MONOTONIC, &old_time);

    int pmfd, pfd;

    size_t savestate_size = 0;

    /* Because we may overwrite our parent state, we must save on a temp file
     * and rename it at the end. Again, we must not allocate any memory, so
     * we store the strings on the stack.
     */
    char temppagemappath[1024];
    char temppagespath[1024];

    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL)) {
        LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s and %s", pagemappath, pagespath);

        unlink(pagemappath);
        pmfd = creat(pagemappath, 0644);

        unlink(pagespath);
        pfd = creat(pagespath, 0644);
    }
    else if (base) {
        LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s", basepagespath);

        pmfd = creat(basepagemappath, 0644);
        pfd = creat(basepagespath, 0644);
    }
    else {
        strcpy(temppagemappath, pagemappath);
        strcpy(temppagespath, pagespath);

        strncat(temppagemappath, ".temp", 1023 - strlen(temppagemappath));
        strncat(temppagespath, ".temp", 1023 - strlen(temppagespath));

        LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s and %s", temppagemappath, temppagespath);

        unlink(temppagemappath);
        pmfd = creat(temppagemappath, 0644);

        unlink(temppagespath);
        pfd = creat(temppagespath, 0644);
    }

    MYASSERT(pmfd != -1)
    MYASSERT(pfd != -1)

    int spmfd = open("/proc/self/pagemap", O_RDONLY);
    MYASSERT(spmfd != -1);

    int crfd = -1;
    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        crfd = open("/proc/self/clear_refs", O_WRONLY);
        MYASSERT(crfd != -1);
    }

    /* Saving the savestate header */
    StateHeader sh;
    int n=0;
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED) {
            if (n >= STATEMAXTHREADS) {
                LOG(LL_ERROR, LCF_CHECKPOINT, "   hit the limit of the number of threads");
                break;
            }
            sh.pthread_ids[n] = thread->pthread_id;
            sh.tids[n] = thread->translated_tid;
            sh.states[n++] = thread->orig_state;
        }
    }
    sh.thread_count = n;
    Utils::writeAll(pmfd, &sh, sizeof(sh));
    savestate_size += sizeof(sh);

    /* Load the parent savestate if any. */
    SaveStateSaving state(pmfd, pfd, spmfd);
    SaveStateLoading parent_state(parentpagemappath, parentpagespath);
    SaveStateLoading base_state(basepagemappath, basepagespath);

    /* Read the memory mapping */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Read the first current area */
    Area area;
    bool not_eof = memMapLayout.getNextArea(&area);

    /* Multiple shared areas can point to the same memory, so we detect and 
     * skip all those duplicate areas.
     * TODO: this code only covers the special case of consecutive areas to 
     * handle Ryujinx, it should be expended to detect any duplicate area */
    Area previous_area;
    previous_area.name[0] = '\0';
    
    while (not_eof) {
        if ((area.flags & Area::AREA_SHARED) && (previous_area.flags & Area::AREA_SHARED) &&
            (0 == strncmp(area.name, previous_area.name, Area::FILENAMESIZE)) &&
            (area.offset == previous_area.offset) && 
            (area.size == previous_area.size)) {
            area.skip = true;    
        }
        savestate_size += writeAnArea(state, area, spmfd, parent_state, base_state, base);
        previous_area = area;
        not_eof = memMapLayout.getNextArea(&area);
    }

    savestate_size += writeSaveFiles(state);

    /* Add the last null (eof) area */
    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    Utils::writeAll(pmfd, &area, sizeof(area));
    savestate_size += sizeof(area);

    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
    }

    if (crfd != 1) {
        close(crfd);
    }

    close(spmfd);

    /* Closing the savestate files */
    close(pmfd);
    close(pfd);

    /* Rename the savestate files */
    if ((Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
        rename(temppagemappath, pagemappath);
        rename(temppagespath, pagespath);
    }

    clock_gettime(CLOCK_MONOTONIC, &new_time);
    delta_time = new_time - old_time;
    LOG(LL_INFO, LCF_CHECKPOINT, "Saved state %d of size %zu in %f seconds", base?0:ss_index, savestate_size, delta_time.tv_sec + ((double)delta_time.tv_nsec) / 1000000000.0);

    if (Global::shared_config.savestate_settings & SharedConfig::SS_FORK) {
        /* Store that we are the child, so that destructors may act differently */
        ThreadManager::setChildFork();

        /* Return the savestate index as status code */
        _exit(base?0:ss_index);
    }
}

/* Write a memory area into the savestate. Returns the size of the area in bytes */
static size_t writeAnArea(SaveStateSaving &state, Area &area, int spmfd, SaveStateLoading &parent_state, SaveStateLoading &base_state, bool base)
{
    if (!(area.prot & PROT_READ)) {
        MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ)) == 0)
    }

    state.processArea(&area);
    size_t area_size = sizeof(area);

    if (area.skip || area.uncommitted) {
        if (!(area.prot & PROT_READ)) {
            MYASSERT(mprotect(area.addr, area.size, area.prot) == 0)
        }

        return area_size;
    }

    area.print("Save");

    /* Seek at the beginning of the area pagemap */
    MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(area.addr) / (4096/8)), SEEK_SET));

    /* Number of pages in the area */
    size_t nb_pages = area.size / 4096;

    /* Index of the current area page */
    size_t page_i = 0;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    /* Stats to print */
    int pagecount_unmapped = 0;
    int pagecount_zero_or_file = 0;
    int pagecount_full = 0;
    int pagecount_base = 0;
    
    /* File descriptor of mapped file, only for debugging */
    int orig_fd = -1;
    
    /* File descriptor of the shared mapped file, to check for holes */
    int shared_fd = -1;
    off_t shared_next_off_hole = -1;
    off_t shared_next_off_data = -1;

    if (area.flags & Area::AREA_SHARED) {
        /* Try first to open the file */
        shared_fd = open(area.name, O_RDONLY);

        /* If no file, shared mappings always have an underlying file accessible
         * inside /proc/self/map_files/ */
        if (shared_fd == -1) {
            shared_fd = open(area.map_file, O_RDONLY);
        }
        
        if (shared_fd != -1) {
            /* Try to seek here */
            shared_next_off_hole = lseek(shared_fd, area.offset, SEEK_HOLE);
            if (shared_next_off_hole == -1) {
                LOG(LL_DEBUG, LCF_CHECKPOINT, "     Could not seek into shared map file %s", area.map_file);
                close(shared_fd);
                shared_fd = -1;
            }
            else {
                shared_next_off_data = lseek(shared_fd, area.offset, SEEK_DATA);
                if (shared_next_off_data == -1) shared_next_off_data = LONG_MAX; // No data, set to past end of file
                shared_next_off_data &= ~0xfffl;
                shared_next_off_hole &= ~0xfffl;
            }
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "     Could not open shared map file %s", area.map_file);
        }
    }

    char* endAddr = static_cast<char*>(area.endAddr);
    for (char* curAddr = static_cast<char*>(area.addr); curAddr < endAddr; curAddr += 4096, page_i++) {

        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if (pagemap_i >= 512) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        /* Gather the flag for the current pagemap. */
        uint64_t page = pagemaps[pagemap_i++];
        bool soft_dirty = page & (0x1ull << 55);
        bool page_file = page & (0x1ull << 61);
        bool page_present = page & (0x1ull << 63);

        if (area.flags & Area::AREA_PRIV) {
            if (area.flags & Area::AREA_ANON) {
                /* Check if page is present */
                if ((Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) && (!page_present)) {
                    state.savePageFlag(Area::NO_PAGE);
                    pagecount_unmapped++;
                    continue;
                }
                
                /* Check if page is zero */
                if (Utils::isZeroPage(static_cast<void*>(curAddr))) {
                    state.savePageFlag(Area::ZERO_PAGE);
                    pagecount_zero_or_file++;
                    continue;
                }
            }
            
            /* Check if page was mapped from file and was not modified since. */
            if ((area.flags & Area::AREA_FILE) && (!page_present || page_file)) {
                state.savePageFlag(Area::FILE_PAGE);
                pagecount_zero_or_file++;

                /* Double-check that page is indeed identical to file */
                // if (Global::shared_config.logging_level >= LL_DEBUG) {
                //     if (orig_fd == -1) {
                //         orig_fd = open(area.name, O_RDONLY);
                //         if (orig_fd == -1)
                //             orig_fd = -2; // don't try again
                //     }
                //     if (orig_fd >= 0) {
                //         lseek(orig_fd, page_i*4096 + area.offset, SEEK_SET);
                //         char buf[4096];
                //         Utils::readAll(orig_fd, buf, 4096);
                //         if (0 != memcmp(curAddr, buf, 4096)) {
                //             LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be identical to file, but is actually not!", curAddr);
                //         }
                //     }
                // }
                continue;
            }
        }

        /* If page from a shared mapping of a file is not present, we can also skip it. */
        if (area.flags & Area::AREA_SHARED) {

            /* Check for a hole in the mapped file (works even for anonymous
             * mappings, which still have an underlying file) */
            if (shared_fd != -1) {
                off_t current_off = area.offset + page_i*4096;

                if ((current_off > shared_next_off_data) && (current_off > shared_next_off_hole)) {
                    /* data and hole offsets are obsolete, update both of them */
                    shared_next_off_data = lseek(shared_fd, current_off, SEEK_DATA);
                    if (shared_next_off_data == -1) shared_next_off_data = LONG_MAX; // No data, set to past end of file
                    shared_next_off_data &= ~0xfffl;
                    shared_next_off_hole = lseek(shared_fd, current_off, SEEK_HOLE);
                    shared_next_off_hole &= ~0xfffl;
                }

                if ((current_off == shared_next_off_data) || (current_off < shared_next_off_hole)) {
                    /* Current page is data */
                }
                else if ((current_off == shared_next_off_hole) || (current_off < shared_next_off_data)) {
                    /* This page contains a hole in the underlying file */
                    state.savePageFlag(Area::NO_PAGE);
                    pagecount_unmapped++;

                    continue;
                }
                else {
                    LOG(LL_WARN, LCF_CHECKPOINT, "     Seeking into shared file shows neither data or hole!");                        
                }
            }

            if (!page_present) {
                state.savePageFlag(Area::FILE_PAGE);
                pagecount_zero_or_file++;
                continue;
            }
            if (Utils::isZeroPage(static_cast<void*>(curAddr))) {
                state.savePageFlag(Area::ZERO_PAGE);
                pagecount_zero_or_file++;
                continue;
            }
        }

        /* Check if page was not modified since last savestate */
        if (!soft_dirty && (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
            /* Copy the value of the parent savestate if any */
            if (parent_state) {
                char parent_flag = parent_state.getPageFlag(curAddr);
                if ((parent_flag == Area::NONE) || (parent_flag == Area::FULL_PAGE) || (parent_flag == Area::COMPRESSED_PAGE)) {
                    /* Parent does not have the page or parent stores the memory page,
                     * saving the full page. */

                    area_size += state.queuePageSave(curAddr);
                    pagecount_full++;
                    continue;
                }
                
                if (parent_flag == Area::ZERO_PAGE) {
                    pagecount_zero_or_file++;
                    
                    /* Double-check that page is indeed zero */
                    if (Global::shared_config.logging_level >= LL_DEBUG) {
                        if (!Utils::isZeroPage(static_cast<void*>(curAddr))) {
                            LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be zero, but is actually not!", curAddr);
                        }
                    }
                }
                else {
                    pagecount_base++;

                    /* Double-check that page is indeed identical to the base savestate */
                    if (Global::shared_config.logging_level >= LL_DEBUG) {
                        char base_flag = base_state.getPageFlag(curAddr);
                        
                        if ((base_flag != Area::FULL_PAGE) && (base_flag != Area::COMPRESSED_PAGE)) {
                            LOG(LL_WARN, LCF_CHECKPOINT, "     No base page for %p, this should not happen!", curAddr);
                        }

                        if (!base_state.debugIsMatchingPage(curAddr)) {
                            LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be identical to base state, but is actually not!", curAddr);                                
                        }
                    }
                }
                state.savePageFlag(parent_flag);
            }
            else {
                state.savePageFlag(Area::BASE_PAGE);
                pagecount_base++;
            }
        }
        else {
            area_size += state.queuePageSave(curAddr);
            pagecount_full++;
        }
    }

    area_size += state.finishSave();

    /* Add the number of page flags to the total size */
    area_size += nb_pages;

    if (!(area.prot & PROT_READ)) {
        MYASSERT(mprotect(area.addr, area.size, area.prot) == 0)
    }

    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d, unmapped: %d, base: %d. Size %zu", pagecount_full, pagecount_zero_or_file, pagecount_unmapped, pagecount_base, area_size);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d, base: %d. Size %zu", pagecount_full, pagecount_zero_or_file, pagecount_base, area_size);
        }
    }
    else {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d, unmapped: %d. Size %zu", pagecount_full, pagecount_zero_or_file, pagecount_unmapped, area_size);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero/file: %d. Size %zu", pagecount_full, pagecount_zero_or_file, area_size);
        }
    }

    if (orig_fd >= 0)
        close(orig_fd);

    if (shared_fd >= 0)
        close(shared_fd);

    return area_size;
}

/* Write a memory area into the savestate. Returns the size of the area in bytes */
static size_t writeSaveFiles(SaveStateSaving &state)
{
    size_t total_size = 0;
    
    for (auto it = SaveFileList::begin(); it != SaveFileList::end(); it++) {
        const std::unique_ptr<SaveFile>& savefile = *it;
        /* Don't map if file was closed and removed (TODO: handle this!)*/
        /* Also, skip if we are handling the savefile using our custom
         * stream (SaveFileStream), which maps its content in memory. So, it
         * is already saved through the memory dumping process. In this case,
         * the fd is set to zero, so the condition below works. */
        if (savefile->fd == 0)
            continue;
            
        Area area;
        area.flags = Area::AREA_SAVEFILE;
        area.fd = savefile->fd;
        strncpy(area.name, savefile->filename.c_str(), Area::FILENAMESIZE-1);
        
        /* Get current file length and map at least this amount of bytes */
        struct stat filestat;
        int rv = fstat(area.fd, &filestat);

        if (rv < 0)
            continue;
        
        area.size = filestat.st_size;
        
        if (area.size == 0) {
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Don't map empty/missing file %s with fd %d", savefile->filename.c_str(), savefile->fd);        
            continue;
        }
        
        area.addr = mmap(nullptr, area.size, PROT_READ, MAP_PRIVATE | MAP_FILE, savefile->fd, 0);
        
        if (area.addr == MAP_FAILED) {
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not map file %s with fd %d and size %zu", savefile->filename.c_str(), savefile->fd, area.size);
            continue;
        }

        LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "Save savefile %s with fd %d and size %zu", savefile->filename.c_str(), savefile->fd, area.size);

        /* Map the original file */
        int orig_fd = -1;
        size_t orig_file_mapped_size = 0;
        void* orig_file_mapped_addr = nullptr;
        
        rv = stat(savefile->filename.c_str(), &filestat);

        if (rv < 0)
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     No original savefile to use");
        else if (! S_ISREG(filestat.st_mode) || (filestat.st_size == 0))
            LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "    Original file is missing or not regular, skip it");
        else {
            orig_fd = open(savefile->filename.c_str(), O_RDONLY);
            
            if (orig_fd == -1)
                LOG(LL_DEBUG, LCF_CHECKPOINT | LCF_FILEIO, "     Cound not locate original savefile");
            else {
                orig_file_mapped_size = filestat.st_size;
                orig_file_mapped_addr = mmap(nullptr, filestat.st_size, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_FILE, orig_fd, 0);
                close(orig_fd);
            }
        }
        
        /* Go through both original and current mapped files, and only store 
         * pages that are different */
        char* mapped_addr_begin = static_cast<char*>(area.addr);
        char* mapped_addr_end = mapped_addr_begin + area.size;
        
        char* orig_file_mapped_begin = static_cast<char*>(orig_file_mapped_addr);
        char* orig_file_mapped_end = orig_file_mapped_begin + orig_file_mapped_size;

        /* Stats to print */
        int pagecount_full = 0;
        int pagecount_file = 0;

        state.processArea(&area);
        size_t area_size = sizeof(area);

        for (;mapped_addr_begin < mapped_addr_end; mapped_addr_begin += 4096, orig_file_mapped_begin += 4096) {
            size_t page_len = (mapped_addr_end - mapped_addr_begin) > 4096 ? 4096 : (mapped_addr_end - mapped_addr_begin);

            /* Check difference with original file */
            if (orig_file_mapped_addr) {
                
                /* If we reached the end of the original file, save all remaining pages */
                if (orig_file_mapped_begin > orig_file_mapped_end) {
                    area_size += state.queuePageSave(mapped_addr_begin);
                    pagecount_full++;
                }
                else {
                    int diff = memcmp(mapped_addr_begin, orig_file_mapped_begin, page_len);
                    if (diff == 0) {
                        state.savePageFlag(Area::FILE_PAGE);
                        pagecount_file++;
                    }
                    else {
                        area_size += state.queuePageSave(mapped_addr_begin);
                        pagecount_full++;
                    }
                }
            }
            else {
                area_size += state.queuePageSave(mapped_addr_begin);
                pagecount_full++;
            }
        }
        
        area_size += state.finishSave();

        /* Add the number of page flags to the total size */
        area_size += (area.size + 4095) / 4096;

        LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, file: %d. Size %zu", pagecount_full, pagecount_file, area_size);

        total_size += area_size;

        /* Unmap files */
        if (area.addr != MAP_FAILED)
            munmap(area.addr, area.size);

        if (orig_file_mapped_addr != MAP_FAILED)
            munmap(orig_file_mapped_addr, filestat.st_size);

    }

    return total_size;
}

}
