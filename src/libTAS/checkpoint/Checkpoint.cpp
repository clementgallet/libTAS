/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../logging.h"
#include "ProcMapsArea.h"
#include "ProcSelfMaps.h"
#include "StateHeader.h"
#include "../Utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <X11/Xlibint.h>
#include <X11/Xlib-xcb.h>
#include <sys/statvfs.h>
#include <sys/syscall.h>
#include "errno.h"
#include "../../external/xcbint.h"
#include "../renderhud/RenderHUD.h"
#include "ReservedMemory.h"
#include "SaveState.h"

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
static int ss_index;
static int parent_ss_index;
static int base_ss_index;

static bool skipArea(const Area *area);

static void readAllAreas();
static int reallocateArea(Area *saved_area, Area *current_area);
static void readAnArea(const Area &saved_area, int pmfd, int pfd, int spmfd, SaveState &parent_state, SaveState &base_state);

static void writeAllAreas(bool base);
static void writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state);

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

void Checkpoint::setParentSavestatePath(std::string path)
{
    std::string pmpath = path + ".pm";
    strncpy(parentpagemappath, pmpath.c_str(), 1023);

    std::string ppath = path + ".p";
    strncpy(parentpagespath, ppath.c_str(), 1023);
}

void Checkpoint::setSavestateIndex(int index)
{
    ss_index = index;
}

void Checkpoint::setBaseSavestateIndex(int index)
{
    base_ss_index = index;
}

void Checkpoint::setParentSavestateIndex(int index)
{
    parent_ss_index = index;
}

static int getPagemapFd(int index)
{
    int* pagemaps = static_cast<int*>(ReservedMemory::getAddr(ReservedMemory::PAGEMAPS_ADDR));
    return pagemaps[index];
}

static int getPagesFd(int index)
{
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

bool Checkpoint::checkCheckpoint()
{
    if (shared_config.savestates_in_ram)
        return true;

    /* Get an estimation of the savestate space */
    size_t savestate_size = 0;

    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(ReservedMemory::PSM_ADDR), ReservedMemory::PSM_SIZE);

    /* Read the first current area */
    Area area;
    bool not_eof = procSelfMaps.getNextArea(&area);

    while (not_eof) {
        if (!skipArea(&area)) {
            savestate_size += area.size;
        }
        not_eof = procSelfMaps.getNextArea(&area);
    }

    /* Get the savestate directory */
    std::string savestate_str = pagemappath;
    size_t sep = savestate_str.find_last_of("/");
    if (sep != std::string::npos)
        savestate_str.resize(sep);

    struct statvfs devData;
    int ret;
    if ((ret = statvfs(savestate_str.c_str(), &devData)) >= 0) {
        unsigned long available_size = devData.f_bavail * devData.f_bsize;
        if (savestate_size > available_size) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Not enough available space to store the savestate");
            RenderHUD::insertMessage("Not enough available space to store the savestate");
            return false;
        }
    }

    return true;
}

bool Checkpoint::checkRestore()
{
    /* Check that the savestate files exist */
    if (shared_config.savestates_in_ram) {
        if (!getPagemapFd(ss_index)) {
            RenderHUD::insertMessage("Savestate does not exist");
            return false;
        }

        if (!getPagesFd(ss_index)) {
            RenderHUD::insertMessage("Savestate does not exist");
            return false;
        }
    }
    else {
        struct stat sb;
        if (stat(pagemappath, &sb) == -1) {
            RenderHUD::insertMessage("Savestate does not exist");
            return false;
        }
        if (stat(pagespath, &sb) == -1) {
            RenderHUD::insertMessage("Savestate does not exist");
            return false;
        }
    }

    int pmfd;
    if (shared_config.savestates_in_ram) {
        pmfd = getPagemapFd(ss_index);
        lseek(pmfd, 0, SEEK_SET);
    }
    else {
        NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
        if (pmfd == -1)
            return false;
    }

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));
    if (!shared_config.savestates_in_ram) {
        NATIVECALL(close(pmfd));
    }

    /* Check that the thread list is identical */
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
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
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
            RenderHUD::insertMessage("Loading the savestate not allowed because new threads were created");
            return false;
        }
    }

    if (n != sh.thread_count) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
        RenderHUD::insertMessage("Loading the savestate not allowed because new threads were created");
        return false;
    }

    return true;
}

void Checkpoint::handler(int signum)
{
    /* Access the X Window identifier from the SDL_Window struct */
    Display *display = gameDisplay;
    if (!display) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Could not access connection to X11");
        return;
    }

    /* Check that we are using our alternate stack by looking at the address
     * of this local variable.
     */
    if ((&display < ReservedMemory::getAddr(0)) ||
        (&display >= ReservedMemory::getAddr(ReservedMemory::getSize()))) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Checkpoint code is not running on alternate stack");
        return;
    }

    XSync(display, false);

    if (ThreadManager::restoreInProgress) {
        /* Before reading from the savestate, we must keep some values from
         * the connection to the X server, because they are used for checking
         * consistency of requests. We can store them in this (alternate) stack
         * which will be preserved.
         */

#ifdef X_DPY_GET_LAST_REQUEST_READ
        uint64_t last_request_read = X_DPY_GET_LAST_REQUEST_READ(display);
        uint64_t request = X_DPY_GET_REQUEST(display);
#else
        uint64_t last_request_read = static_cast<uint64_t>(display->last_request_read);
        uint64_t request = static_cast<uint64_t>(display->request);
#endif

        /* Copy the entire xcb connection struct */
        xcb_connection_t xcb_conn;
        xcb_connection_t *cur_xcb_conn = XGetXCBConnection(display);
        memcpy(&xcb_conn, cur_xcb_conn, sizeof(xcb_connection_t));

        readAllAreas();
        /* restoreInProgress was overwritten, putting the right value again */
        ThreadManager::restoreInProgress = true;

        /* Restoring the display values */
#ifdef X_DPY_SET_LAST_REQUEST_READ
        X_DPY_SET_LAST_REQUEST_READ(display, last_request_read);
        X_DPY_SET_REQUEST(display, request);
#else
        display->last_request_read = static_cast<unsigned long>(last_request_read);
        display->request = static_cast<unsigned long>(request);
#endif

        /* Restore the entire xcb connection struct */
        memcpy(cur_xcb_conn, &xcb_conn, sizeof(xcb_connection_t));
    }
    else {
        /* Check that base savestate exists, otherwise save it */
        if (shared_config.incremental_savestates) {
            if (shared_config.savestates_in_ram) {
                int fd = getPagemapFd(base_ss_index);
                if (!fd) {
                    writeAllAreas(true);
                }
            }
            else {
                struct stat sb;
                if (stat(basepagemappath, &sb) == -1) {
                    writeAllAreas(true);
                }
            }
        }

        writeAllAreas(false);
    }
    debuglogstdio(LCF_CHECKPOINT, "End restore.");
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

    if (strstr(area->name, "(deleted)")) {
        return true;
    }

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

    /* Don't save shared memory */
    if (area->flags & MAP_SHARED) {
        return true;
    }

    /* Save area if write permission */
    if (area->prot & PROT_WRITE) {
        return false;
    }

    /* Save anonymous area even if write protection off, because some
     * games or libs could change protections
     */
    if (area->flags & MAP_ANONYMOUS) {
        return false;
    }

    return true;
}

static void readAllAreas()
{
    int pmfd;
    if (shared_config.savestates_in_ram) {
        pmfd = getPagemapFd(ss_index);
        lseek(pmfd, 0, SEEK_SET);
    }
    else {
        NATIVECALL(pmfd = open(pagemappath, O_RDONLY));
        MYASSERT(pmfd != -1)
    }

    int pfd;
    if (shared_config.savestates_in_ram) {
        pfd = getPagesFd(ss_index);
        lseek(pfd, 0, SEEK_SET);
    }
    else {
        NATIVECALL(pfd = open(pagespath, O_RDONLY));
        MYASSERT(pfd != -1)
    }

    int spmfd, crfd;
    if (shared_config.incremental_savestates) {
        NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
        MYASSERT(spmfd != -1);

        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
        MYASSERT(crfd != -1);
    }

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(pmfd, &sh, sizeof(sh));

    Area current_area;
    Area saved_area;

    debuglogstdio(LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(ReservedMemory::PSM_ADDR), ReservedMemory::PSM_SIZE);

    /* Read the first saved area */
    Utils::readAll(pmfd, &saved_area, sizeof(Area));
    if (!saved_area.skip)
        lseek(pmfd, saved_area.size / 4096, SEEK_CUR);

    /* Read the first current area */
    bool not_eof = procSelfMaps.getNextArea(&current_area);

    /* Reallocate areas to match the savestate areas. Nothing is written yet */
    while ((saved_area.addr != nullptr) || not_eof) {

        /* Check for matching areas */
        int cmp = reallocateArea(&saved_area, &current_area);
        if (cmp == 0) {
            /* Areas matched, we advance both areas */
            Utils::readAll(pmfd, &saved_area, sizeof(Area));
            if (!saved_area.skip)
                lseek(pmfd, saved_area.size / 4096, SEEK_CUR);
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp > 0) {
            /* Saved area is greater, advance current area */
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp < 0) {
            /* Saved area is smaller, advance saved area */
            Utils::readAll(pmfd, &saved_area, sizeof(Area));
            if (!saved_area.skip)
                lseek(pmfd, saved_area.size / 4096, SEEK_CUR);
        }
    }

    /* Now that the memory layout matches the savestate, we load savestate into memory */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);

    /* Load base and parent savestates */
    SaveState parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));
    SaveState base_state(basepagemappath, basepagespath, getPagemapFd(base_ss_index), getPagesFd(base_ss_index));

    /* Read the first saved area */
    Utils::readAll(pmfd, &saved_area, sizeof saved_area);

    while (saved_area.addr != nullptr) {
        readAnArea(saved_area, pmfd, pfd, spmfd, parent_state, base_state);
        Utils::readAll(pmfd, &saved_area, sizeof(saved_area));
    }

    if (shared_config.incremental_savestates) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
    }

    /* Recover write permission to all areas */
    lseek(pmfd, sizeof(StateHeader), SEEK_SET);

    Utils::readAll(pmfd, &saved_area, sizeof saved_area);
    if (!saved_area.skip)
        lseek(pmfd, saved_area.size / 4096, SEEK_CUR);

    while (saved_area.addr != nullptr) {
        if (!saved_area.skip) {
            MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
        }
        Utils::readAll(pmfd, &saved_area, sizeof(saved_area));
        if (!saved_area.skip)
            lseek(pmfd, saved_area.size / 4096, SEEK_CUR);
    }

    if (shared_config.incremental_savestates) {
        NATIVECALL(close(crfd));
        NATIVECALL(close(spmfd));
    }

    /* Closing savestate files */
    if (!shared_config.savestates_in_ram) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }
}

static int reallocateArea(Area *saved_area, Area *current_area)
{
    saved_area->print("Restore");
    current_area->print("Current");

    /* Do Areas start on the same address? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
        (saved_area->addr == current_area->addr)) {

        /* Check if it is a skipped area */
        if (saved_area->skip) {
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Current section is not skipped anymore !?");
                saved_area->print("Saved");
                current_area->print("Current");
            }
            return 0;
        }

        size_t copy_size = (saved_area->size<current_area->size)?saved_area->size:current_area->size;

        if (saved_area->size != current_area->size) {

            /* Special case for stacks, always try to resize the Area */
            if (strstr(saved_area->name, "[stack")) {
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

                copy_size = saved_area->size;
            }

            /* Special case for heap, use brk instead */
            if (strcmp(saved_area->name, "[heap]") == 0) {
                debuglogstdio(LCF_CHECKPOINT, "Changing heap size from %d to %d", current_area->size, saved_area->size);

                int ret = brk(saved_area->endAddr);

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

        if ((saved_area->addr != nullptr) && (current_area->endAddr > saved_area->addr)) {
            /* Areas are overlapping, we only unmap the non-shared region */
            ptrdiff_t unmap_size = reinterpret_cast<ptrdiff_t>(saved_area->addr) - reinterpret_cast<ptrdiff_t>(current_area->addr);

            /* We only deallocate this section if we are interested in it */
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, unmap_size);
                MYASSERT(munmap(current_area->addr, unmap_size) == 0)
            }

            /* We call this function again with the rest of the area */
            current_area->addr = saved_area->addr;
            current_area->size -= unmap_size;
            return reallocateArea(saved_area, current_area);
        }
        else {
            /* Areas are not overlapping, we unmap the whole area */
            /* We only deallocate this section if we are interested in it */
            if (!skipArea(current_area)) {
                current_area->print("Deallocating");
                MYASSERT(munmap(current_area->addr, current_area->size) == 0)
            }
            return 1;
        }
    }

    if ((current_area->addr == nullptr) || (saved_area->addr < current_area->addr)) {

        if (saved_area->skip) {
            // debuglogstdio(LCF_CHECKPOINT, "Section is skipped");
            return -1;
        }

        size_t map_size = saved_area->size;
        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* Areas are overlapping, we only map the non-shared region */
            map_size = reinterpret_cast<ptrdiff_t>(current_area->addr) - reinterpret_cast<ptrdiff_t>(saved_area->addr);
        }

        /* This saved area must be allocated */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be allocated", saved_area->addr, saved_area->name, map_size);

        int imagefd = -1;
        if (!(saved_area->flags & MAP_ANONYMOUS)) {
            /* We shouldn't be creating any special section such as [heap] or [stack] */
            MYASSERT(saved_area->name[0] == '/')

            NATIVECALL(imagefd = open(saved_area->name, O_RDONLY, 0));
            if (imagefd >= 0) {
                /* If the current file size is smaller than the original, we map the region
                * as private anonymous. Note that with this we lose the name of the region
                * but most applications may not care.
                */
                off_t curr_size = lseek(imagefd, 0, SEEK_END);
                MYASSERT(curr_size != -1);
                if (static_cast<size_t>(curr_size) < saved_area->offset + map_size) {
                    NATIVECALL(close(imagefd));
                    imagefd = -1;
                    saved_area->offset = 0;
                    saved_area->flags |= MAP_ANONYMOUS;
                }
            }
            else {
                /* We could not open the file, map the section as anonymous */
                saved_area->flags |= MAP_ANONYMOUS;
            }
        }

        if (saved_area->flags & MAP_ANONYMOUS) {
            debuglogstdio(LCF_CHECKPOINT, "Restoring anonymous area, %d bytes at %p", map_size, saved_area->addr);
        } else {
            debuglogstdio(LCF_CHECKPOINT, "Restoring non-anonymous area, %d bytes at %p from %s + %d", map_size, saved_area->addr, saved_area->name, saved_area->offset);
        }

        /* Create the memory area */

        void *mmappedat = mmap(saved_area->addr, map_size, saved_area->prot,
            saved_area->flags, imagefd, saved_area->offset);

        if (mmappedat == MAP_FAILED) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Mapping %d bytes at %p failed", saved_area->size, saved_area->addr);
            return -1;
        }
        if (mmappedat != saved_area->addr) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
            return -1;
        }

        /* Close image file (fd only gets in the way) */
        if (imagefd >= 0) {
            NATIVECALL(close(imagefd));
        }

        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* If areas were overlapping, we must deal with the rest of the area */
            saved_area->addr = current_area->addr;
            saved_area->size -= map_size;
            return reallocateArea(saved_area, current_area);
        }

        return -1;
    }

    MYASSERT(false)
    return 0;
}

static void readAnArea(const Area &saved_area, int pmfd, int pfd, int spmfd, SaveState &parent_state, SaveState &base_state)
{
    if (saved_area.skip)
        return;

    MYASSERT(saved_area.page_offset == lseek(pfd, 0, SEEK_CUR))

    /* Add write permission to the area */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot | PROT_WRITE) == 0)
    }

    if (shared_config.incremental_savestates) {
        /* Seek at the beginning of the area pagemap */
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

    /* Chunk of savestate pagemap values */
    char ss_pagemaps[4096];

    /* Current index in the savestate pagemap array */
    int ss_pagemap_i = 4096;

    char* endAddr = static_cast<char*>(saved_area.endAddr);
    for (char* curAddr = static_cast<char*>(saved_area.addr);
    curAddr < endAddr;
    curAddr += 4096, page_i++, pagemap_i++) {

        /* We read savestate pagemap flags in chunks to avoid too many read syscalls. */
        if (ss_pagemap_i >= 4096) {
            size_t remaining_pages = ((nb_pages-page_i)>4096)?4096:(nb_pages-page_i);
            Utils::readAll(pmfd, ss_pagemaps, remaining_pages);
            ss_pagemap_i = 0;
        }

        /* Same for pagemap file */
        if (shared_config.incremental_savestates && (pagemap_i >= 512)) {
            size_t remaining_pages = ((nb_pages-page_i)>512)?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        char flag = ss_pagemaps[ss_pagemap_i++];

        if (flag == Area::NO_PAGE) {
        }
        else if (flag == Area::ZERO_PAGE) {
            memset(static_cast<void*>(curAddr), 0, 4096);
        }
        else if (flag == Area::BASE_PAGE) {
            /* The memory page of the loading savestate is the same as the base
             * savestate. We must check if we actually need to read from the
             * base savestate or if the page already contains the correct values.
             */

            char parent_flag = parent_state.getPageFlag(curAddr);
            parent_state.skipPage(parent_flag);

            if (parent_flag != Area::BASE_PAGE) {
                /* Memory page has been modified between the two savestates.
                 * We must read from the base savestate.
                 */
                char base_flag = base_state.getPageFlag(curAddr);
                MYASSERT(base_flag == Area::FULL_PAGE);
                Utils::readAll(base_state.getPageFd(), static_cast<void*>(curAddr), 4096);
            }
            else {
                /* Gather the flag for the page map */
                uint64_t page = pagemaps[pagemap_i];
                bool soft_dirty = page & (0x1ull << 55);

                if (soft_dirty) {
                    /* Memory page has been modified after parent state.
                     * We must read from the base savestate.
                     */
                    char base_flag = base_state.getPageFlag(curAddr);
                    MYASSERT(base_flag == Area::FULL_PAGE);
                    Utils::readAll(base_state.getPageFd(), static_cast<void*>(curAddr), 4096);
                }
            }
        }
        else {
            Utils::readAll(pfd, static_cast<void*>(curAddr), 4096);
        }
    }

    /* Recover permission to the area */
    if (!(saved_area.prot & PROT_WRITE)) {
        MYASSERT(mprotect(saved_area.addr, saved_area.size, saved_area.prot) == 0)
    }
}


static void writeAllAreas(bool base)
{
    int pmfd, pfd;

    /* Storing the savestate index in stack */
    int current_ss_index = ss_index;

    /* Because we may overwrite our parent state, we must save on a temp file
     * and rename it at the end. Again, we must not allocate any memory, so
     * we store the strings on the stack.
     */
    char temppagemappath[1024];
    char temppagespath[1024];

    if (shared_config.savestates_in_ram) {
        if (!shared_config.incremental_savestates) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in slot ", ss_index);

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
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s", base_ss_index);

            /* Create new memfds */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            setPagemapFd(base_ss_index, pmfd);

            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
            setPagesFd(base_ss_index, pfd);
        }
        else {
            /* Creating new memfds for temp state */
            pmfd = syscall(SYS_memfd_create, "pagemapstate", 0);
            pfd = syscall(SYS_memfd_create, "pagesstate", 0);
        }
    }
    else {
        if (!shared_config.incremental_savestates) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s and %s", pagemappath, pagespath);

            unlink(pagemappath);
            NATIVECALL(pmfd = creat(pagemappath, 0644));

            unlink(pagespath);
            NATIVECALL(pfd = creat(pagespath, 0644));
        }
        else if (base) {
            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s", basepagespath);

            NATIVECALL(pmfd = creat(basepagemappath, 0644));
            NATIVECALL(pfd = creat(basepagespath, 0644));
        }
        else {
            strncpy(temppagemappath, pagemappath, 1023);
            strncpy(temppagespath, pagespath, 1023);

            strncat(temppagemappath, ".temp", 1023 - strlen(temppagemappath));
            strncat(temppagespath, ".temp", 1023 - strlen(temppagespath));

            debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s and %s", temppagemappath, temppagespath);

            unlink(temppagemappath);
            NATIVECALL(pmfd = creat(temppagemappath, 0644));

            unlink(temppagespath);
            NATIVECALL(pfd = creat(temppagespath, 0644));
        }
    }

    MYASSERT(pmfd != -1)
    MYASSERT(pfd != -1)

    int spmfd;
    NATIVECALL(spmfd = open("/proc/self/pagemap", O_RDONLY));
    MYASSERT(spmfd != -1);

    int crfd;
    if (shared_config.incremental_savestates) {
        NATIVECALL(crfd = open("/proc/self/clear_refs", O_WRONLY));
        MYASSERT(crfd != -1);
    }

    /* Saving the savestate header */
    StateHeader sh;
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED) {
            sh.pthread_ids[n] = thread->pthread_id;
            sh.tids[n++] = thread->tid;
        }
    }
    sh.thread_count = n;
    Utils::writeAll(pmfd, &sh, sizeof(sh));

    /* Load the parent savestate if any. */
    SaveState parent_state(parentpagemappath, parentpagespath, getPagemapFd(parent_ss_index), getPagesFd(parent_ss_index));

    /* Parse the content of /proc/self/maps into memory.
     * We don't allocate memory here, we are using our special allocated
     * memory section that won't be saved in the savestate.
     */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(ReservedMemory::PSM_ADDR), ReservedMemory::PSM_SIZE);

    /* Remove write and add read flags from all memory areas we will be dumping */
    Area area;
    while (procSelfMaps.getNextArea(&area)) {
        if (!skipArea(&area)) {
            //MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ) & ~PROT_WRITE) == 0)
            MYASSERT(mprotect(area.addr, area.size, (area.prot | PROT_READ)) == 0)
            MYASSERT(madvise(area.addr, area.size, MADV_SEQUENTIAL) == 0);
        }
    }

    /* Dump all memory areas */
    procSelfMaps.reset();
    while (procSelfMaps.getNextArea(&area)) {
        if (skipArea(&area)) {
            area.skip = true;
            Utils::writeAll(pmfd, &area, sizeof(area));
        }
        else {
            writeAnArea(pmfd, pfd, spmfd, area, parent_state);
        }
    }

    /* Add the last null (eof) area */
    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    Utils::writeAll(pmfd, &area, sizeof(area));

    if (shared_config.incremental_savestates) {
        /* Clear soft-dirty bits */
        Utils::writeAll(crfd, "4\n", 2);
    }

    /* Recover area protection and advise */
    procSelfMaps.reset();
    while (procSelfMaps.getNextArea(&area)) {
        if (!skipArea(&area)) {
            MYASSERT(mprotect(area.addr, area.size, area.prot) == 0)
            MYASSERT(madvise(area.addr, area.size, MADV_NORMAL) == 0);
        }
    }

    if (shared_config.incremental_savestates) {
        NATIVECALL(close(crfd));
    }

    NATIVECALL(close(spmfd));

    /* Closing the savestate files */
    if (!shared_config.savestates_in_ram) {
        NATIVECALL(close(pmfd));
        NATIVECALL(close(pfd));
    }

    /* Rename the savestate files */
    if (shared_config.incremental_savestates && !base) {
        if (shared_config.savestates_in_ram) {
            /* Closing the old savestate memfds and replace with the new one */
            if (getPagemapFd(current_ss_index)) {
                NATIVECALL(close(getPagemapFd(current_ss_index)));
                NATIVECALL(close(getPagesFd(current_ss_index)));
            }
            setPagemapFd(current_ss_index, pmfd);
            setPagesFd(current_ss_index, pfd);
        }
        else {
            rename(temppagemappath, pagemappath);
            rename(temppagespath, pagespath);
        }
    }
}

static void writeAnArea(int pmfd, int pfd, int spmfd, Area &area, SaveState &parent_state)
{
//    area.print("Save");

    /* Save the position of the first area page in the pages file */
    area.page_offset = lseek(pfd, 0, SEEK_CUR);
    MYASSERT(area.page_offset != -1)

    /* Write the area struct */
    Utils::writeAll(pmfd, &area, sizeof(area));

    /* Seek at the beginning of the area pagemap */
    MYASSERT(-1 != lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(area.addr) / (4096/8)), SEEK_SET));

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

    char* endAddr = static_cast<char*>(area.endAddr);
    for (char* curAddr = static_cast<char*>(area.addr); curAddr < endAddr; curAddr += 4096, page_i++) {

        /* We write a chunk of savestate pagemaps if it is full */
        if (ss_pagemap_i >= 4096) {
            Utils::writeAll(pmfd, ss_pagemaps, 4096);
            ss_pagemap_i = 0;
        }

        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if (pagemap_i >= 512) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }

        /* Gather the flag for the current pagemap. */
        uint64_t page = pagemaps[pagemap_i++];
        bool page_present = page & (0x1ull << 63);
        bool soft_dirty = page & (0x1ull << 55);

        /* Check if page is present */
        if (!page_present) {
            ss_pagemaps[ss_pagemap_i++] = Area::NO_PAGE;
        }

        /* Check if page is zero (only check on anonymous memory)*/
        else if ((area.flags & MAP_ANONYMOUS) && Utils::isZeroPage(static_cast<void*>(curAddr))) {
            ss_pagemaps[ss_pagemap_i++] = Area::ZERO_PAGE;
        }

        /* Check if page was not modified since last savestate */
        else if (!soft_dirty && shared_config.incremental_savestates) {
            /* Copy the value of the parent savestate if any */
            if (parent_state) {
                char parent_flag = parent_state.getPageFlag(curAddr);

                if (parent_flag == Area::NONE) {
                    /* This is not supposed to happen, saving the full page */
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area with soft-dirty cleared but no parent page !?");
                    ss_pagemaps[ss_pagemap_i++] = Area::FULL_PAGE;
                    Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
                }
                else if (parent_flag == Area::FULL_PAGE) {
                    /* Parent state stores the memory page, we must store it too */
                    ss_pagemaps[ss_pagemap_i++] = Area::FULL_PAGE;
                    Utils::writeAll(pfd, parent_state.getPage(parent_flag), 4096);
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
            ss_pagemaps[ss_pagemap_i++] = Area::FULL_PAGE;
            Utils::writeAll(pfd, static_cast<void*>(curAddr), 4096);
        }
    }

    /* Writing the last savestate pagemap chunk */
    Utils::writeAll(pmfd, ss_pagemaps, ss_pagemap_i);

}

}
