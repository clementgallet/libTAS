/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../logging.h"
#include "MemArea.h"

#ifdef __unix__
#include "ProcSelfMaps.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "MachVmMaps.h"
#endif

#include "StateHeader.h"
#include "../Utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <stdint.h>
#include <sys/statvfs.h>
#include "errno.h"
#include "../renderhud/RenderHUD.h"
#include "ReservedMemory.h"
#include "SaveState.h"
#include "../../external/lz4.h"
#include "../../shared/sockethelpers.h"

#ifdef __unix__
#include <X11/Xlibint.h>
#include <X11/Xlib-xcb.h>
#include "../../external/xcbint.h"
#include "../xlib/xdisplay.h" // x11::gameDisplays
#endif

#ifdef __linux__
#include <sys/syscall.h>
#endif

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

static bool skipArea(const Area *area);

static void readAllAreas();
static int reallocateArea(Area *saved_area, Area *current_area);
static void readAnArea(SaveState &saved_area, int spmfd, SaveState &parent_state, SaveState &base_state);

static void writeAllAreas(bool base);
static size_t writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state, bool base);

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
    if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
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

static int getPagemapFd(int index)
{
    if (index < 0) return 0;
    int* pagemaps = static_cast<int*>(ReservedMemory::getAddr(ReservedMemory::PAGEMAPS_ADDR));
    return pagemaps[index];
}

static int getPagesFd(int index)
{
    if (index < 0) return 0;
    int* pages = static_cast<int*>(ReservedMemory::getAddr(ReservedMemory::PAGES_ADDR));
    return pages[index];
}

static void setPagemapFd(int index, int fd)
{
    int* pagemaps = static_cast<int*>(ReservedMemory::getAddr(ReservedMemory::PAGEMAPS_ADDR));
    pagemaps[index] = fd;
}
static void setPagesFd(int index, int fd)
{
    int* pages = static_cast<int*>(ReservedMemory::getAddr(ReservedMemory::PAGES_ADDR));
    pages[index] = fd;
}

int Checkpoint::checkCheckpoint()
{
    if (shared_config.savestate_settings & SharedConfig::SS_RAM)
        return SaveStateManager::ESTATE_OK;

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
        if (!skipArea(&area)) {
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
    if (shared_config.savestate_settings & SharedConfig::SS_RAM) {
        if (!getPagemapFd(ss_index)) {
            return SaveStateManager::ESTATE_NOSTATE;
        }

        if (!getPagesFd(ss_index)) {
            return SaveStateManager::ESTATE_NOSTATE;
        }
    }
    else {
        struct stat sb;
        if (stat(pagemappath, &sb) == -1) {
            return SaveStateManager::ESTATE_NOSTATE;
        }
        if (stat(pagespath, &sb) == -1) {
            return SaveStateManager::ESTATE_NOSTATE;
        }
    }

    int pmfd;
    if (shared_config.savestate_settings & SharedConfig::SS_RAM) {
        pmfd = getPagemapFd(ss_index);
        lseek(pmfd, 0, SEEK_SET);
    }
    else {
        NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
        if (pmfd == -1)
            return SaveStateManager::ESTATE_NOSTATE;
    }

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));
    if (!(shared_config.savestate_settings & SharedConfig::SS_RAM)) {
        NATIVECALL(close(pmfd));
    }

    /* Check that the thread list is identical */
    int n=0;
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if ((thread->state != ThreadInfo::ST_RUNNING) &&
            (thread->state != ThreadInfo::ST_ZOMBIE) &&
            (thread->state != ThreadInfo::ST_FREE))
            continue;

        int t;
        for (t=0; t<sh.thread_count; t++) {
            if (sh.pthread_ids[t] == thread->pthread_id && (sh.tids[t] == thread->tid)) {
                n++;
                break;
            }
        }

        if (t == sh.thread_count) {
            /* We didn't find a match */
            return SaveStateManager::ESTATE_NOTSAMETHREADS;
        }
    }

    if (n != sh.thread_count) {
        return SaveStateManager::ESTATE_NOTSAMETHREADS;
    }

    return SaveStateManager::ESTATE_OK;
}

void Checkpoint::handler(int signum)
{
#ifdef __unix__
    /* Check that we are using our alternate stack by looking at the address
     * of this local variable.
     */
    Display *display = x11::gameDisplays[0];
    if ((&display < ReservedMemory::getAddr(0)) ||
        (&display >= ReservedMemory::getAddr(ReservedMemory::getSize()))) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Checkpoint code is not running on alternate stack");
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
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &old_time));
        readAllAreas();
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &new_time));
        delta_time = new_time - old_time;
        debuglogstdio(LCF_CHECKPOINT | LCF_INFO, "Loaded state %d in %f seconds", ss_index, delta_time.tv_sec + ((double)delta_time.tv_nsec) / 1000000000.0);

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

        /* We must restore the current stack frame from the savestate */
        AltStack::restoreStackFrame();
    }
    else {
        /* Check that base savestate exists, otherwise save it */
        if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
            if (shared_config.savestate_settings & SharedConfig::SS_RAM) {
                int fd = getPagemapFd(base_ss_index);
                if (!fd) {
                    writeAllAreas(true);
                }
            }
            else {
                struct stat sb;
                if (stat(basepagemappath, &sb) == -1) {
                    resetParent();
                    writeAllAreas(true);
                }
            }
        }
        /* We must store the current stack frame in the savestate */
        AltStack::saveStackFrame();

        writeAllAreas(false);
    }
}

static bool skipArea(const Area *area)
{
    /* If it's readable, but it's VDSO, it will be dangerous to restore it.
    * In 32-bit mode later Red Hat RHEL Linux 2.6.9 releases use 0xffffe000,
    * the last page of virtual memory.  Note 0xffffe000 >= HIGHEST_VA
    * implies we're in 32-bit mode.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (void*)0xffffe000) {
        return true;
    }
#ifdef __x86_64__

    /* And in 64-bit mode later Red Hat RHEL Linux 2.6.9 releases
    * use 0xffffffffff600000 for VDSO.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (void*)0xffffffffff600000) {
        return true;
    }
#endif // ifdef __x86_64__

    if (area->size == 0) {
        /* Kernel won't let us munmap this.  But we don't need to restore it. */
        return true;
    }

    if (0 == strcmp(area->name, "[vsyscall]") ||
    0 == strcmp(area->name, "[vectors]") ||
    0 == strcmp(area->name, "[vvar]") ||
    0 == strcmp(area->name, "[vdso]")) {
        return true;
    }

    /* Don't save our reserved memory */
    if ((area->addr == ReservedMemory::getAddr(0)) && (area->size == ReservedMemory::getSize())) {
        return true;
    }

    /* Don't save area that cannot be promoted to read/write */
    if ((area->max_prot & (PROT_WRITE|PROT_READ)) != (PROT_WRITE|PROT_READ)) {
        return false;
    }
    
    /* Save area if write permission */
    if (area->prot & PROT_WRITE) {
        return false;
    }

    /* Save anonymous area even if write protection off, because some
     * games or libs could change protections
     */
    if (area->flags & Area::AREA_ANON) {
        return false;
    }

    return true;
}

static void readAllAreas()
{
    SaveState saved_state(pagemappath, pagespath, getPagemapFd(ss_index), getPagesFd(ss_index));

    int spmfd = -1;
    if (shared_config.savestate_settings & (SharedConfig::SS_INCREMENTAL | SharedConfig::SS_PRESENT)) {
        NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
        MYASSERT(spmfd != -1);
    }

    int crfd = -1;
    if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
        MYASSERT(crfd != -1);
    }

    /* Read the savestate header */
    StateHeader sh;
    saved_state.readHeader(sh);

    Area current_area;
    Area saved_area = saved_state.getArea();

    debuglogstdio(LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Read the first current area */
    bool not_eof = memMapLayout.getNextArea(&current_area);

    /* Reallocate areas to match the savestate areas. Nothing is written yet */
    while ((saved_area.addr != nullptr) || not_eof) {

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
    saved_area = saved_state.nextArea();
    
    /* Load base and parent savestates */
    SaveState parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));
    SaveState base_state(basepagemappath, basepagespath, getPagemapFd(base_ss_index), getPagesFd(base_ss_index));

    /* If the loading savestate and the parent savestate are the same, pass the
     * same SaveState object to readAnArea because two SaveState objects
     * handling the same file descriptor will mess up the file offset. */
    bool same_state = (ss_index == parent_ss_index);
    while (saved_area.addr != nullptr) {
        readAnArea(saved_state, spmfd, same_state?saved_state:parent_state, base_state);
        saved_area = saved_state.nextArea();
    }

    if (crfd != -1) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
        NATIVECALL(close(crfd));
    }

    if (spmfd != -1) {
        NATIVECALL(close(spmfd));
    }
}

static int reallocateArea(Area *saved_area, Area *current_area)
{
    saved_area->print("Restore");
    current_area->print("Current");

    /* Do Areas start on the same address? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
        (saved_area->addr == current_area->addr)) {

        /* If it is not the same area, unmap the current area */
        if ((strcmp(saved_area->name, current_area->name) != 0) ||
            (saved_area->flags != current_area->flags)) {

            debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
            MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            return 1;
        }

        size_t copy_size = (saved_area->size<current_area->size)?saved_area->size:current_area->size;

        if (saved_area->size != current_area->size) {

            /* Special case for stacks, always try to resize the Area */
            if (saved_area->flags & Area::AREA_STACK) {
#ifdef __linux__
                debuglogstdio(LCF_CHECKPOINT, "Changing stack size from %d to %d", current_area->size, saved_area->size);
                void *newAddr = mremap(current_area->addr, current_area->size, saved_area->size, 0);

                if (newAddr == MAP_FAILED) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Resizing failed");
                    return 0;
                }

                if (newAddr != saved_area->addr) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "mremap relocated the area");
                    return 0;
                }
#else
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "stack size changed but mremap is not available");
#endif
                copy_size = saved_area->size;
            }

            /* Special case for heap, use brk instead */
            if (saved_area->flags & Area::AREA_HEAP) {
                debuglogstdio(LCF_CHECKPOINT, "Changing heap size from %d to %d", current_area->size, saved_area->size);

#ifdef __linux__
                int ret = brk(saved_area->endAddr);
#else
                intptr_t ret = reinterpret_cast<intptr_t>(brk(saved_area->endAddr));
#endif

                if (ret < 0) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "brk failed");
                    return 0;
                }

                copy_size = saved_area->size;
            }
        }

        /* Apply the protections from the saved area if needed */
        if (saved_area->prot != current_area->prot) {
            MYASSERT(mprotect(saved_area->addr, copy_size, saved_area->prot) == 0)
        }

        if ((saved_area->endAddr == current_area->endAddr) || (saved_area->name[0] == '[')) {
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

    if ((saved_area->addr == nullptr) || (saved_area->addr > current_area->addr)) {
        /* Our current area starts before the saved area */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
        MYASSERT(munmap(current_area->addr, current_area->size) == 0)

        return 1;
    }

    if ((current_area->addr == nullptr) || (saved_area->addr < current_area->addr)) {

        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* Areas are overlapping, we unmap the current area until there is no more overlapping */
            debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
            MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            return 1;
        }

        /* This saved area must be allocated */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be allocated", saved_area->addr, saved_area->name, saved_area->size);

        int imagefd = -1;
        if (saved_area->flags & Area::AREA_FILE) {
            /* We shouldn't be creating any special section such as [heap] or [stack] */
            MYASSERT(saved_area->name[0] == '/')

            NATIVECALL(imagefd = open(saved_area->name, O_RDWR, 0));
            if (imagefd >= 0) {
                /* If the current file size is smaller than the original, we map the region
                * as private anonymous. Note that with this we lose the name of the region
                * but most applications may not care.
                */
                off_t curr_size = lseek(imagefd, 0, SEEK_END);
                if ((curr_size != -1) && (static_cast<size_t>(curr_size) < saved_area->offset + saved_area->size)) {
                    NATIVECALL(close(imagefd));
                    imagefd = -1;
                    saved_area->offset = 0;
                    saved_area->flags = Area::AREA_ANON;
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
            debuglogstdio(LCF_CHECKPOINT, "Restoring anonymous area, %d bytes at %p", saved_area->size, saved_area->addr);
        } else {
            debuglogstdio(LCF_CHECKPOINT, "Restoring non-anonymous area, %d bytes at %p from %s + %d", saved_area->size, saved_area->addr, saved_area->name, saved_area->offset);
        }

        /* Create the memory area */
        void *mmappedat = mmap(saved_area->addr, saved_area->size, saved_area->prot,
                               saved_area->toMmapFlag(), imagefd, saved_area->offset);

        if (mmappedat == MAP_FAILED) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Mapping %d bytes at %p failed: errno %d", saved_area->size, saved_area->addr, errno);
        }

        if (mmappedat != saved_area->addr) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
        }

        /* Close image file (fd only gets in the way) */
        if (imagefd >= 0) {
            NATIVECALL(close(imagefd));
        }

        return -1;
    }

    MYASSERT(false)
    return 0;
}

static void readAnArea(SaveState &saved_state, int spmfd, SaveState &parent_state, SaveState &base_state)
{
    const Area& saved_area = saved_state.getArea();

    if (saved_area.skip)
        return;

    /* Add write permission to the area */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot | PROT_WRITE) == 0)
    }

    if (spmfd != -1) {
        MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(saved_area.addr) / (4096/8)), SEEK_SET));
    }

    /* Number of pages in the area */
    int nb_pages = saved_area.size / 4096;

    /* Index of the current area page */
    int page_i = 0;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    char* endAddr = static_cast<char*>(saved_area.endAddr);
    for (char* curAddr = static_cast<char*>(saved_area.addr);
    curAddr < endAddr;
    curAddr += 4096, page_i++, pagemap_i++) {

        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if ((spmfd != -1) && (pagemap_i >= 512)) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        char flag = saved_state.getNextPageFlag();

        /* Gather the flag for the page map */
        uint64_t page = (spmfd != -1)?pagemaps[pagemap_i++]:-1;
        bool soft_dirty = page & (0x1ull << 55);
        bool page_present = page & (0x1ull << 63);

        /* It seems that static memory is both zero and unmapped, so we still
         * need to memset the region if it was mapped.
         *
         * TODO: This setting breaks savestates in Freedom Planet, when saving
         * on one stage, advancing to the next stage, loading the state and 
         * advancing to next stage again. */
        if (flag == Area::NO_PAGE) {
            if (page_present && (!Utils::isZeroPage(static_cast<void*>(curAddr))))
                memset(static_cast<void*>(curAddr), 0, 4096);
        }
        else if (flag == Area::ZERO_PAGE) {
            if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
                /* In case incremental savestates is enabled, we can guess that
                 * the page is already zero if the parent page is zero and the
                 * page was not modified since. In that case, we can skip the memset. */
                if (soft_dirty ||
                    parent_state.getPageFlag(curAddr) != Area::ZERO_PAGE) {
                    memset(static_cast<void*>(curAddr), 0, 4096);
                }
            }
            else {
                /* Only memset if the page is not zero, to prevent an actual
                 * allocation if the page was allocated but never used. */
                if (!Utils::isZeroPage(static_cast<void*>(curAddr))) {
                    memset(static_cast<void*>(curAddr), 0, 4096);
                }
            }
        }
        else if (flag == Area::BASE_PAGE) {
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
            }
            else {
                if (soft_dirty) {
                    /* Memory page has been modified after parent state.
                     * We must read from the base savestate.
                     */
                    base_state.getPageFlag(curAddr);
                    base_state.queuePageLoad(curAddr);
                }
            }
        }
        else {
            saved_state.queuePageLoad(curAddr);
        }
    }
    base_state.finishLoad();
    saved_state.finishLoad();

    /* Recover permission to the area */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
    }
}


static void writeAllAreas(bool base)
{
    if (shared_config.savestate_settings & SharedConfig::SS_FORK) {
        pid_t pid;
        NATIVECALL(pid = fork());
        if (pid != 0)
            return;

        ThreadManager::restoreThreadTids();
    }

    TimeHolder old_time, new_time, delta_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &old_time));

    int pmfd, pfd;

    size_t savestate_size = 0;

    /* Storing the savestate index in stack */
    int current_ss_index = ss_index;

    /* Because we may overwrite our parent state, we must save on a temp file
     * and rename it at the end. Again, we must not allocate any memory, so
     * we store the strings on the stack.
     */
    char temppagemappath[1024];
    char temppagespath[1024];

#ifdef __linux__
    if (shared_config.savestate_settings & SharedConfig::SS_RAM) {
        if (!(shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL)) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in slot %d", ss_index);

            pmfd = getPagemapFd(ss_index);
            if (pmfd) {
                ftruncate(pmfd, 0);
                lseek(pmfd, 0, SEEK_SET);
            }
            else {
                /* Create a new memfd */
                pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
                setPagemapFd(ss_index, pmfd);
            }

            pfd = getPagesFd(ss_index);
            if (pfd) {
                ftruncate(pfd, 0);
                lseek(pfd, 0, SEEK_SET);
            }
            else {
                /* Create a new memfd */
                pfd = syscall(SYS_memfd_create, "pagesstate", 0);
                setPagesFd(ss_index, pfd);
            }
        }
        else if (base) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in slot %d", base_ss_index);

            /* Create new memfds */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            setPagemapFd(base_ss_index, pmfd);

            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
            setPagesFd(base_ss_index, pfd);
        }
        else {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in slot %d", ss_index);
            /* Creating new memfds for temp state */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
        }
    }
    else
#endif
    {
        if (!(shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL)) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s and %s", pagemappath, pagespath);

            NATIVECALL(unlink(pagemappath));
            NATIVECALL(pmfd = creat(pagemappath, 0644));

            NATIVECALL(unlink(pagespath));
            NATIVECALL(pfd = creat(pagespath, 0644));
        }
        else if (base) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s", basepagespath);

            NATIVECALL(pmfd = creat(basepagemappath, 0644));
            NATIVECALL(pfd = creat(basepagespath, 0644));
        }
        else {
            strcpy(temppagemappath, pagemappath);
            strcpy(temppagespath, pagespath);

            strncat(temppagemappath, ".temp", 1023 - strlen(temppagemappath));
            strncat(temppagespath, ".temp", 1023 - strlen(temppagespath));

            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s and %s", temppagemappath, temppagespath);

            NATIVECALL(unlink(temppagemappath));
            NATIVECALL(pmfd = creat(temppagemappath, 0644));

            NATIVECALL(unlink(temppagespath));
            NATIVECALL(pfd = creat(temppagespath, 0644));
        }
    }

    MYASSERT(pmfd != -1)
    MYASSERT(pfd != -1)

    int spmfd = -1;
    NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
    if (shared_config.savestate_settings & (SharedConfig::SS_INCREMENTAL | SharedConfig::SS_PRESENT)) {
        /* We need /proc/self/pagemap for incremental savestates */
        MYASSERT(spmfd != -1);
    }

    int crfd = -1;
    if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
        MYASSERT(crfd != -1);
    }

    /* Saving the savestate header */
    StateHeader sh;
    int n=0;
    for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED) {
            if (n >= STATEMAXTHREADS) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "   hit the limit of the number of threads");
                break;
            }
            sh.pthread_ids[n] = thread->pthread_id;
            sh.tids[n++] = thread->tid;
        }
    }
    sh.thread_count = n;
    Utils::writeAll(pmfd, &sh, sizeof(sh));
    savestate_size += sizeof(sh);

    /* Load the parent savestate if any. */
    SaveState parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));

    /* Parse the memory mapping layout.
     * We don't allocate memory here, we are using our special allocated
     * memory section that won't be saved in the savestate.
     */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Remove write and add read flags from all memory areas we will be dumping */
    Area area;
    while (memMapLayout.getNextArea(&area)) {
        if (!skipArea(&area)) {
            //MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ) & ~PROT_WRITE) == 0)
//            debuglogstdio(LCF_CHECKPOINT, "   mprotect on addr %p", area.addr);

            MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ)) == 0)
            MYASSERT(madvise(area.addr, area.size, MADV_SEQUENTIAL) == 0);
        }
    }

    /* Dump all memory areas */
    memMapLayout.reset();
    while (memMapLayout.getNextArea(&area)) {
        savestate_size += writeAnArea(pmfd, pfd, spmfd, area, parent_state, base);
    }

    /* Add the last null (eof) area */
    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    Utils::writeAll(pmfd, &area, sizeof(area));
    savestate_size += sizeof(area);

    if (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
    }

    /* Recover area protection and advise */
    memMapLayout.reset();
    while (memMapLayout.getNextArea(&area)) {
        if (!skipArea(&area)) {
            MYASSERT(mprotect(area.addr, area.size, area.prot) == 0)
            MYASSERT(madvise(area.addr, area.size, MADV_NORMAL) == 0);
        }
    }

    if (crfd != 1) {
        NATIVECALL(close(crfd));
    }

    if (spmfd != -1) {
        NATIVECALL(close(spmfd));
    }

    /* Closing the savestate files */
    if (!(shared_config.savestate_settings & SharedConfig::SS_RAM)) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }

    /* Rename the savestate files */
    if ((shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
        if (shared_config.savestate_settings & SharedConfig::SS_RAM) {
            /* Closing the old savestate memfds and replace with the new one */
            if (getPagemapFd(current_ss_index)) {
                NATIVECALL(close(getPagemapFd(current_ss_index)));
                NATIVECALL(close(getPagesFd(current_ss_index)));
            }
            setPagemapFd(current_ss_index, pmfd);
            setPagesFd(current_ss_index, pfd);
        }
        else {
            NATIVECALL(rename(temppagemappath, pagemappath));
            NATIVECALL(rename(temppagespath, pagespath));
        }
    }

    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &new_time));
    delta_time = new_time - old_time;
    debuglogstdio(LCF_INFO, "Saved state %d of size %zu in %f seconds", base?0:ss_index, savestate_size, delta_time.tv_sec + ((double)delta_time.tv_nsec) / 1000000000.0);

    if (shared_config.savestate_settings & SharedConfig::SS_FORK) {
        /* Store that we are the child, so that destructors may act differently */
        ThreadManager::setChildFork();

        /* Return the savestate index as status code */
        _exit(base?0:ss_index);
    }
}

/* Write a memory area into the savestate. Returns the size of the area in bytes */
static size_t writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state, bool base)
{
    area.print("Save");
    size_t area_size = 0;

    /* Save the position of the first area page in the pages file */
    area.page_offset = lseek(pfd, 0, SEEK_CUR);
    MYASSERT(area.page_offset != -1)

    /* Write the area struct */
    area.skip = skipArea(&area);
    Utils::writeAll(pmfd, &area, sizeof(area));
    area_size += sizeof(area);

    if (area.skip)
        return area_size;

    if (spmfd != -1) {
        /* Seek at the beginning of the area pagemap */
        MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(area.addr) / (4096/8)), SEEK_SET));
    }

    /* Number of pages in the area */
    int nb_pages = area.size / 4096;

    /* Index of the current area page */
    int page_i = 0;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    /* Chunk of savestate pagemap values */
    char ss_pagemaps[4096];

    /* Current index in the savestate pagemap array */
    int ss_pagemap_i = 0;

    /* Compressed chunk */
    char compressed_page[LZ4_COMPRESSBOUND(4096)];

    char* endAddr = static_cast<char*>(area.endAddr);
    for (char* curAddr = static_cast<char*>(area.addr); curAddr < endAddr; curAddr += 4096, page_i++) {

        /* We write a chunk of savestate pagemaps if it is full */
        if (ss_pagemap_i >= 4096) {
            Utils::writeAll(pmfd, ss_pagemaps, 4096);
            ss_pagemap_i = 0;
            area_size += 4096;
        }

        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if ((spmfd != -1) && (pagemap_i >= 512)) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        /* Gather the flag for the current pagemap. */
        uint64_t page = (spmfd != -1)?pagemaps[pagemap_i++]:-1;
        bool page_present = page & (0x1ull << 63);
        bool soft_dirty = page & (0x1ull << 55);

        /* Check if page is present */
        if ((shared_config.savestate_settings & SharedConfig::SS_PRESENT) && (!page_present)) {
            ss_pagemaps[ss_pagemap_i++] = Area::NO_PAGE;
        }

        /* Check if page is zero (only check on anonymous memory)*/
        else if ((area.flags & Area::AREA_ANON) && Utils::isZeroPage(static_cast<void*>(curAddr))) {
            ss_pagemaps[ss_pagemap_i++] = Area::ZERO_PAGE;
        }

        /* Check if page was not modified since last savestate */
        else if (!soft_dirty && (shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
            /* Copy the value of the parent savestate if any */
            if (parent_state) {
                char parent_flag = parent_state.getPageFlag(curAddr);
                if ((parent_flag == Area::NONE) || (parent_flag == Area::FULL_PAGE) || (parent_flag == Area::COMPRESSED_PAGE)) {
                    /* Parent does not have the page or parent stores the memory page,
                     * saving the full page. */

                    int compressed_size = 0;
                    if (shared_config.savestate_settings & SharedConfig::SS_COMPRESSED) {
                        compressed_size = LZ4_compress_default(curAddr, compressed_page, 4096, LZ4_COMPRESSBOUND(4096));
                    }
                    if (compressed_size != 0) {
                        ss_pagemaps[ss_pagemap_i++] = Area::COMPRESSED_PAGE;
                        Utils::writeAll(pfd, &compressed_size, sizeof(int));
                        Utils::writeAll(pfd, compressed_page, compressed_size);
                        area_size += compressed_size;
                    }
                    else {
                        ss_pagemaps[ss_pagemap_i++] = Area::FULL_PAGE;
                        Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
                        area_size += 4096;
                    }
                }
                else {
                    ss_pagemaps[ss_pagemap_i++] = parent_flag;
                }
            }
            else {
                ss_pagemaps[ss_pagemap_i++] = Area::BASE_PAGE;
            }
        }
        else {
            int compressed_size = 0;
            if (shared_config.savestate_settings & SharedConfig::SS_COMPRESSED) {
                compressed_size = LZ4_compress_default(curAddr, compressed_page, 4096, LZ4_COMPRESSBOUND(4096));
            }
            if (compressed_size != 0) {
                ss_pagemaps[ss_pagemap_i++] = Area::COMPRESSED_PAGE;
                Utils::writeAll(pfd, &compressed_size, sizeof(int));
                Utils::writeAll(pfd, compressed_page, compressed_size);
                area_size += compressed_size;
            }
            else {
                ss_pagemaps[ss_pagemap_i++] = Area::FULL_PAGE;
                Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
                area_size += 4096;
            }
        }
    }

    /* Writing the last savestate pagemap chunk */
    Utils::writeAll(pmfd, ss_pagemaps, ss_pagemap_i);
    area_size += ss_pagemap_i;

    return area_size;
}

}
