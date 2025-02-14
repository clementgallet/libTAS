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
#include "TimeHolder.h"

#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#include "Utils.h"
#include "renderhud/RenderHUD.h"
#include "../shared/sockethelpers.h"
#ifdef __unix__
#include "../external/xcbint.h"
#include "xlib/xdisplay.h" // x11::gameDisplays
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
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

static void writeAllAreas(bool base);
static size_t writeAnArea(SaveStateSaving &state, Area &area, int spmfd, SaveStateLoading &parent_state, SaveStateLoading &base_state, bool base);

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
    if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM)
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
        if (!area.isSkipped()) {
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
    if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM) {
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
    if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM) {
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
    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_RAM)) {
        NATIVECALL(close(pmfd));
    }

    return SaveStateManager::ESTATE_OK;
}

void Checkpoint::handler(int signum, siginfo_t *info, void *ucontext)
{
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
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &old_time));
        readAllAreas();
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &new_time));
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
            if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM) {
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
    SaveStateLoading saved_state(pagemappath, pagespath, getPagemapFd(ss_index), getPagesFd(ss_index));
    saved_state.readHeader(sh);
}

static void readAllAreas()
{
    SaveStateLoading saved_state(pagemappath, pagespath, getPagemapFd(ss_index), getPagesFd(ss_index));

    int spmfd = -1;
    if (Global::shared_config.savestate_settings & (SharedConfig::SS_INCREMENTAL | SharedConfig::SS_PRESENT)) {
        NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
        MYASSERT(spmfd != -1);
    }

    int crfd = -1;
    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
        MYASSERT(crfd != -1);
    }

    /* Read the savestate header */
    StateHeader sh;
    saved_state.readHeader(&sh);

    Area current_area;
    Area saved_area = saved_state.getArea();

    LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Read the first current area */
    bool not_eof = memMapLayout.getNextArea(&current_area);

    /* Reallocate areas to match the savestate areas. Nothing is written yet */
    while (saved_area || not_eof) {

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
    
    /* Load base and parent savestates */
    SaveStateLoading parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));
    SaveStateLoading base_state(basepagemappath, basepagespath, getPagemapFd(base_ss_index), getPagesFd(base_ss_index));

    /* If the loading savestate and the parent savestate are the same, pass the
     * same SaveStateLoading object to readAnArea because two SaveStateLoading objects
     * handling the same file descriptor will mess up the file offset. */
    bool same_state = (ss_index == parent_ss_index);
    while (saved_area) {
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
    /* Do Areas start on the same address? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
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

            /* Special case for memfd savefiles, always try to resize the Area */
            if (saved_area->flags & Area::AREA_MEMFD) {
#ifdef __linux__
                LOG(LL_DEBUG, LCF_CHECKPOINT, "Changing memfd size from %d to %d", current_area->size, saved_area->size);
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
                LOG(LL_FATAL, LCF_CHECKPOINT, "memfd size changed but mremap is not available");
#endif
                copy_size = saved_area->size;
            }
        }

        /* Apply the protections from the saved area if needed */
        if (saved_area->prot != current_area->prot) {
            MYASSERT(mprotect(saved_area->addr, copy_size, saved_area->prot) == 0)
        }

        if ((saved_area->endAddr == current_area->endAddr) ||
            (saved_area->name[0] == '[') || 
            (saved_area->flags & Area::AREA_MEMFD)) {
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

        if (saved_area->flags & Area::AREA_MEMFD) {
            /* Special case for memfd savefiles, we must mmap memfd file descriptor */
            if (saved_area->memfd_fd >= 0) {
                LOG(LL_DEBUG, LCF_CHECKPOINT, "Restoring memfd area, %d bytes at %p with file fd %d and size %zu", saved_area->size, saved_area->addr, saved_area->memfd_fd, saved_area->memfd_size);

                void *mmappedat = mmap(saved_area->addr, saved_area->size, saved_area->prot,
                                       MAP_SHARED, saved_area->memfd_fd, 0);

                if (mmappedat == MAP_FAILED) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "Mapping %d bytes at %p failed: errno %d", saved_area->size, saved_area->addr, errno);
                }

                if (mmappedat != saved_area->addr) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
                }
            }
            else {
                LOG(LL_DEBUG, LCF_CHECKPOINT, "Restoring memfd area but no file descriptor, allocate anonymous area");

                void *mmappedat = mmap(saved_area->addr, saved_area->size, saved_area->prot,
                                       MAP_ANONYMOUS, 0, 0);

                if (mmappedat == MAP_FAILED) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "Mapping %d bytes at %p failed: errno %d", saved_area->size, saved_area->addr, errno);
                }

                if (mmappedat != saved_area->addr) {
                    LOG(LL_FATAL, LCF_CHECKPOINT, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
                }                
            }

            return -1;
        }

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

        /* Close image file (fd only gets in the way) */
        if (imagefd >= 0) {
            NATIVECALL(close(imagefd));
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

    if (spmfd != -1) {
        MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(saved_area.addr) / (4096/8)), SEEK_SET));
    }

    /* Number of pages in the area */
    size_t nb_pages = saved_area.size / 4096;

    /* Index of the current area page */
    size_t page_i = 0;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    /* Stats to print */
    int pagecount_unmapped = 0;
    int pagecount_zero = 0;
    int pagecount_full = 0;
    int pagecount_base = 0;
    int pagecount_skip = 0;

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

        char flag = saved_area.uncommitted ? Area::NO_PAGE : saved_state.getNextPageFlag();

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
            pagecount_unmapped++;
            if (page_present && (!Utils::isZeroPage(static_cast<void*>(curAddr)))) {
                if (!(saved_area.prot & PROT_WRITE)) {
                    MYASSERT(mprotect(curAddr, 4096, saved_area.prot | PROT_WRITE | PROT_READ) == 0)
                }
                memset(static_cast<void*>(curAddr), 0, 4096);
            }
        }
        else if (flag == Area::ZERO_PAGE) {
            pagecount_zero++;
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
                }
                else {
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
                }
            }
        }
        else {
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
                }
                else {
                    if (soft_dirty) {
                        /* Memory page has been modified after parent state.
                         * We must read from the base savestate.
                         */
                        base_state.getPageFlag(curAddr);
                        base_state.queuePageLoad(curAddr);
                        pagecount_base++;
                    }
                    else {
                        pagecount_skip++;
                        /* Double-check that page is indeed identical to the base savestate */
                        if (Global::shared_config.logging_level >= LL_DEBUG) {
                            base_state.getPageFlag(curAddr);
                            if (!base_state.debugIsMatchingPage(curAddr)) {
                                LOG(LL_WARN, LCF_CHECKPOINT, "     Page %p was guessed to be identical to base state, but is actually not!", curAddr);                                
                            }
                        }

                    }
                }
            }
            else {
                pagecount_full++;
                saved_state.queuePageLoad(curAddr);
            }
        }
    }
    base_state.finishLoad();
    saved_state.finishLoad();

    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, unmapped: %d, base: %d, skipped: %d", pagecount_full, pagecount_zero, pagecount_unmapped, pagecount_base, pagecount_skip);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, base: %d, skipped: %d", pagecount_full, pagecount_zero, pagecount_base, pagecount_skip);
        }
    }
    else {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, unmapped: %d", pagecount_full, pagecount_zero, pagecount_unmapped);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d", pagecount_full, pagecount_zero);
        }
    }

    saved_state.checkHash();

    /* Recover permission to the area */
    if (!(saved_area.prot & PROT_WRITE) || !(saved_area.prot & PROT_READ)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
    }
}


static void writeAllAreas(bool base)
{
    if (Global::shared_config.savestate_settings & SharedConfig::SS_FORK) {
        pid_t pid;
        NATIVECALL(pid = fork());
        if (pid != 0)
            return;

        ThreadManager::restoreTid();
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
    if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM) {
        if (!(Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL)) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in slot %d", ss_index);

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
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in slot %d", base_ss_index);

            /* Create new memfds */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            setPagemapFd(base_ss_index, pmfd);

            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
            setPagesFd(base_ss_index, pfd);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in slot %d", ss_index);
            /* Creating new memfds for temp state */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
        }
    }
    else
#endif
    {
        if (!(Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL)) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s and %s", pagemappath, pagespath);

            NATIVECALL(unlink(pagemappath));
            NATIVECALL(pmfd = creat(pagemappath, 0644));

            NATIVECALL(unlink(pagespath));
            NATIVECALL(pfd = creat(pagespath, 0644));
        }
        else if (base) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s", basepagespath);

            NATIVECALL(pmfd = creat(basepagemappath, 0644));
            NATIVECALL(pfd = creat(basepagespath, 0644));
        }
        else {
            strcpy(temppagemappath, pagemappath);
            strcpy(temppagespath, pagespath);

            strncat(temppagemappath, ".temp", 1023 - strlen(temppagemappath));
            strncat(temppagespath, ".temp", 1023 - strlen(temppagespath));

            LOG(LL_DEBUG, LCF_CHECKPOINT, "Performing checkpoint in %s and %s", temppagemappath, temppagespath);

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
    if (Global::shared_config.savestate_settings & (SharedConfig::SS_INCREMENTAL | SharedConfig::SS_PRESENT)) {
        /* We need /proc/self/pagemap for incremental savestates */
        MYASSERT(spmfd != -1);
    }

    int crfd = -1;
    if (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) {
        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
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
    SaveStateLoading parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));
    SaveStateLoading base_state(basepagemappath, basepagespath, getPagemapFd(base_ss_index), getPagesFd(base_ss_index));

    /* Read the memory mapping */
#ifdef __unix__
    ProcSelfMaps memMapLayout;
#elif defined(__APPLE__) && defined(__MACH__)
    MachVmMaps memMapLayout;
#endif

    /* Read the first current area */
    Area area;
    bool not_eof = memMapLayout.getNextArea(&area);
    
    while (not_eof) {
        savestate_size += writeAnArea(state, area, spmfd, parent_state, base_state, base);
        not_eof = memMapLayout.getNextArea(&area);
    }

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
        NATIVECALL(close(crfd));
    }

    if (spmfd != -1) {
        NATIVECALL(close(spmfd));
    }

    /* Closing the savestate files */
    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_RAM)) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }

    /* Rename the savestate files */
    if ((Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_RAM) {
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

    if (spmfd != -1) {
        /* Seek at the beginning of the area pagemap */
        MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(area.addr) / (4096/8)), SEEK_SET));
    }

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
    int pagecount_zero = 0;
    int pagecount_full = 0;
    int pagecount_base = 0;
    
    char* endAddr = static_cast<char*>(area.endAddr);
    for (char* curAddr = static_cast<char*>(area.addr); curAddr < endAddr; curAddr += 4096, page_i++) {

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
        if ((Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) && (!page_present)) {
            state.savePageFlag(Area::NO_PAGE);
            pagecount_unmapped++;
        }

        /* Check if page is zero (only check on anonymous memory)*/
        else if ((area.flags & Area::AREA_ANON) && Utils::isZeroPage(static_cast<void*>(curAddr))) {
            state.savePageFlag(Area::ZERO_PAGE);
            pagecount_zero++;
        }

        /* Check if page was not modified since last savestate */
        else if (!soft_dirty && (Global::shared_config.savestate_settings & SharedConfig::SS_INCREMENTAL) && !base) {
            /* Copy the value of the parent savestate if any */
            if (parent_state) {
                char parent_flag = parent_state.getPageFlag(curAddr);
                if ((parent_flag == Area::NONE) || (parent_flag == Area::FULL_PAGE) || (parent_flag == Area::COMPRESSED_PAGE)) {
                    /* Parent does not have the page or parent stores the memory page,
                     * saving the full page. */

                    area_size += state.queuePageSave(curAddr);
                    pagecount_full++;
                }
                else {
                    if (parent_flag == Area::ZERO_PAGE) {
                        pagecount_zero++;
                        
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
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, unmapped: %d, base: %d. Size %zu", pagecount_full, pagecount_zero, pagecount_unmapped, pagecount_base, area_size);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, base: %d. Size %zu", pagecount_full, pagecount_zero, pagecount_base, area_size);
        }
    }
    else {
        if (Global::shared_config.savestate_settings & SharedConfig::SS_PRESENT) {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d, unmapped: %d. Size %zu", pagecount_full, pagecount_zero, pagecount_unmapped, area_size);
        }
        else {
            LOG(LL_DEBUG, LCF_CHECKPOINT, "    Pagecount full: %d, zero: %d. Size %zu", pagecount_full, pagecount_zero, area_size);
        }
    }

    return area_size;
}

}
