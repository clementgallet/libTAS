/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "Utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <X11/Xlibint.h>
#include "../sdlwindows.h"
#include "ReservedMemory.h"

#define ONE_MB 1024 * 1024

namespace libtas {

static const char* savestatepath;

void Checkpoint::init()
{
    /* Register a signal so that we can switch stacks */
    struct sigaction sigusr2;
    sigfillset(&sigusr2.sa_mask);
    sigusr2.sa_flags = SA_ONSTACK;
    sigusr2.sa_handler = handler;
    {
        GlobalOwnCode goc;
        MYASSERT(sigaction(SIGUSR2, &sigusr2, nullptr) == 0)
    }
}

void Checkpoint::setSavestatePath(const char* filepath)
{
    /* We want to avoid any allocated memory here, so using char array */
    savestatepath = filepath;
}

bool Checkpoint::checkRestore()
{
    /* Check that the savestate file exists */
    struct stat sb;
    if (stat(savestatepath, &sb) == -1)
        return false;

    int fd;
    OWNCALL(fd = open(savestatepath, O_RDONLY));
    if (fd == -1)
        return false;

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(fd, &sh, sizeof(sh));
    close(fd);

    /* Check that the thread list is identical */
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
        if (thread->state != ThreadInfo::ST_RUNNING)
            continue;

        int t;
        for (t=0; t<sh.thread_count; t++) {
            if (sh.thread_tids[t] == thread->tid) {
                n++;
                break;
            }
        }

        if (t == sh.thread_count) {
            /* We didn't find a match */
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
            return false;
        }
    }

    if (n != sh.thread_count) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR | LCF_ALERT, "Loading this state is not supported because the thread list has changed since, sorry");
        return false;
    }

    return true;
}

static bool skipArea(Area *area)
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

    if ((area->addr == ReservedMemory::getAddr(0)) && (area->size == ReservedMemory::getSize())) {
        return true;
    }

    /* Start of user-configurable skips */

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_NON_WRITEABLE) &&
        !(area->prot & PROT_WRITE)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_NON_ANONYMOUS_NON_WRITEABLE) &&
        !(area->prot & PROT_WRITE) && !(area->flags & MAP_ANONYMOUS)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_EXEC) &&
        (area->prot & PROT_EXEC)) {
        return true;
    }

    if ((shared_config.ignore_sections & SharedConfig::IGNORE_SHARED) &&
        (area->flags & MAP_SHARED)) {
        return true;
    }

    return false;
}

/* This function returns a range of zero or non-zero pages. If the first page
 * is non-zero, it searches for all contiguous non-zero pages and returns them.
 * If the first page is all-zero, it searches for contiguous zero pages and
 * returns them.
 */
static void getNextPageRange(Area &area, size_t &size, bool &is_zero)
{
    static const size_t page_size = sysconf(_SC_PAGESIZE);
    static const size_t block_size = 25 * page_size; // Arbitrary, about 100 KB

    if (area.size < block_size) {
        size = area.size;
        is_zero = false;
        return;
    }

    intptr_t endAddrInt = reinterpret_cast<intptr_t>(area.endAddr);

    size = block_size;
    is_zero = Utils::areZeroPages(area.addr, block_size / page_size);

    // prevAddr = area.addr;
    for (intptr_t curAddrInt = reinterpret_cast<intptr_t>(area.addr) + block_size;
    curAddrInt < endAddrInt;
    curAddrInt += block_size) {

        size_t minsize = ((endAddrInt-curAddrInt)<block_size)?(endAddrInt-curAddrInt):block_size;
        if (is_zero != Utils::areZeroPages(reinterpret_cast<void*>(curAddrInt), minsize / page_size)) {
            break;
        }
        size += minsize;
    }

    /* If we are near the end and processing non-zero memory, we can return
     * the entire section.
     */
    if (((area.size - size) < block_size) && !is_zero)
        size = area.size;
}

static void writeAnAreaWithZeroPages(int fd, Area *orig_area)
{
    Area area = *orig_area;

    while (area.size > 0) {
        size_t size;
        bool is_zero;
        Area a = area;
        getNextPageRange(a, size, is_zero);

        a.properties = is_zero ? Area::ZERO_PAGE : Area::NONE;
        a.size = size;
        void* endAddr = reinterpret_cast<void*>(reinterpret_cast<intptr_t>(area.addr) + size);
        a.endAddr = endAddr;

        Utils::writeAll(fd, &a, sizeof(a));
        if (!is_zero) {
            debuglogstdio(LCF_CHECKPOINT, "Found non zero pages starting %p of size %d", a.addr, a.size);
            Utils::writeAll(fd, a.addr, a.size);
        }
        else {
            debuglogstdio(LCF_CHECKPOINT, "Found zero pages starting %p of size %d", a.addr, a.size);
        }
        area.addr = endAddr;
        area.size -= size;
    }
}

static void writeAnArea(int fd, Area *area)
{
    area->print("Save");

    if ((area->prot & PROT_READ) == 0) {
        MYASSERT(mprotect(area->addr, area->size, area->prot | PROT_READ) == 0)
    }

    if (area->flags & MAP_ANONYMOUS) {
        /* We look for zero pages in anonymous sections and skip saving them */
        writeAnAreaWithZeroPages(fd, area);
    }
    else {
        Utils::writeAll(fd, area, sizeof(*area));
        Utils::writeAll(fd, area->addr, area->size);
    }

    if ((area->prot & PROT_READ) == 0) {
        MYASSERT(mprotect(area->addr, area->size, area->prot) == 0)
    }
}

static void writeAllAreas()
{
    debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint in %s", savestatepath);
    int fd;
    unlink(savestatepath);
    OWNCALL(fd = creat(savestatepath, 0644));
    MYASSERT(fd != -1)

    /* Saving the savestate header */
    StateHeader sh;
    int n=0;
    for (ThreadInfo *thread = ThreadManager::thread_list; thread != nullptr; thread = thread->next) {
        if (thread->state == ThreadInfo::ST_SUSPENDED)
            sh.thread_tids[n++] = thread->tid;
    }
    sh.thread_count = n;
    Utils::writeAll(fd, &sh, sizeof(sh));

    /* Parse the content of /proc/self/maps into memory.
     * We don't allocate memory here, we are using our special allocated
     * memory section that won't be saved in the savestate.
     */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(0), ONE_MB);

    Area area;
    while (procSelfMaps.getNextArea(&area)) {

        if (skipArea(&area)) {
            area.properties |= Area::SKIP;
            Utils::writeAll(fd, &area, sizeof(area));
            continue;
        }

        writeAnArea(fd, &area);
    }

    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    write(fd, &area, sizeof(area));

    /* That's all folks */
    MYASSERT(close(fd) == 0);
}


static int readAndCompAreas(int fd, Area *saved_area, Area *current_area)
{
    /* Do Areas start on the same address? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
        (saved_area->addr == current_area->addr)) {

        /* Check if it is a skipped area */
        if (saved_area->properties & Area::SKIP) {
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Current section is not skipped anymore !?");
                saved_area->print("Saved");
                current_area->print("Current");
            }
            return 0;
        }

        saved_area->print("Restore");

        size_t copy_size = (saved_area->size<current_area->size)?saved_area->size:current_area->size;

        if (saved_area->size != current_area->size) {

            /* Special case for stacks, always try to resize the Area */
            if (strstr(saved_area->name, "[stack")) {
                debuglogstdio(LCF_CHECKPOINT, "Changing stack size from %d to %d", current_area->size, saved_area->size);
                void *newAddr = mremap(current_area->addr, current_area->size, saved_area->size, 0);

                if (newAddr == MAP_FAILED) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Resizing failed");
                    lseek(fd, saved_area->size, SEEK_CUR);
                    return 0;
                }

                if (newAddr != saved_area->addr) {
                    debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "mremap relocated the area");
                    lseek(fd, saved_area->size, SEEK_CUR);
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
                    lseek(fd, saved_area->size, SEEK_CUR);
                    return 0;
                }

                copy_size = saved_area->size;
            }
        }

        /* Now copy the data */
        if (!(current_area->prot & PROT_WRITE)) {
            MYASSERT(mprotect(current_area->addr, copy_size, current_area->prot | PROT_WRITE) == 0)
        }

        if (saved_area->properties & Area::ZERO_PAGE) {
            debuglogstdio(LCF_CHECKPOINT, "Writing %d zero bytes to memory!", copy_size);
            memset(saved_area->addr, 0, copy_size);
        }
        else {
            debuglogstdio(LCF_CHECKPOINT, "Writing %d bytes to memory!", copy_size);
            Utils::readAll(fd, saved_area->addr, copy_size);
        }

        if (!(current_area->prot & PROT_WRITE)) {
            MYASSERT(mprotect(current_area->addr, copy_size, current_area->prot) == 0)
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
            return readAndCompAreas(fd, saved_area, current_area);
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

        if (saved_area->properties & Area::SKIP) {
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

            imagefd = open(saved_area->name, O_RDONLY, 0);
            if (imagefd >= 0) {
                /* If the current file size is smaller than the original, we map the region
                * as private anonymous. Note that with this we lose the name of the region
                * but most applications may not care.
                */
                off_t curr_size = lseek(imagefd, 0, SEEK_END);
                MYASSERT(curr_size != -1);
                if (static_cast<size_t>(curr_size) < saved_area->offset + map_size) {
                    close(imagefd);
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

        void *mmappedat = mmap(saved_area->addr, map_size, saved_area->prot | PROT_WRITE,
            saved_area->flags, imagefd, saved_area->offset);

        if (mmappedat == MAP_FAILED) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Mapping %d bytes at %p failed", saved_area->size, saved_area->addr);
            lseek(fd, map_size, SEEK_CUR);
            return -1;
        }
        if (mmappedat != saved_area->addr) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
            lseek(fd, map_size, SEEK_CUR);
            return -1;
        }

        /* Close image file (fd only gets in the way) */
        if (imagefd >= 0) {
            close(imagefd);
        }

        if (saved_area->properties & Area::ZERO_PAGE) {
            memset(saved_area->addr, 0, map_size);
        }
        else {
            Utils::readAll(fd, saved_area->addr, map_size);
        }

        if (!(saved_area->prot & PROT_WRITE)) {
            MYASSERT(mprotect(saved_area->addr, map_size, saved_area->prot) == 0)
        }

        if ((current_area->addr != nullptr) && (saved_area->endAddr > current_area->addr)) {
            /* If areas were overlapping, we must deal with the rest of the area */
            saved_area->addr = current_area->addr;
            saved_area->size -= map_size;
            return readAndCompAreas(fd, saved_area, current_area);
        }

        return -1;
    }

    MYASSERT(false)
    return 0;
}


static void readAllAreas()
{
    int fd;
    OWNCALL(fd = open(savestatepath, O_RDONLY));
    MYASSERT(fd != -1)

    /* Read the savestate header */
    StateHeader sh;
    Utils::readAll(fd, &sh, sizeof(sh));

    Area current_area;
    Area saved_area;

    debuglogstdio(LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
    ProcSelfMaps procSelfMaps(ReservedMemory::getAddr(0), ONE_MB);

    /* Read the first saved area */
    Utils::readAll(fd, &saved_area, sizeof saved_area);

    /* Read the first current area */
    bool not_eof = procSelfMaps.getNextArea(&current_area);

    while ((saved_area.addr != nullptr) || not_eof) {

        /* Check for matching areas */
        int cmp = readAndCompAreas(fd, &saved_area, &current_area);
        if (cmp == 0) {
            /* Areas matched, we advance both areas */
            Utils::readAll(fd, &saved_area, sizeof saved_area);
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp > 0) {
            /* Saved area is greater, advance current area */
            not_eof = procSelfMaps.getNextArea(&current_area);
        }
        if (cmp < 0) {
            /* Saved area is smaller, advance saved area */
            Utils::readAll(fd, &saved_area, sizeof saved_area);
        }
    }

    /* That's all folks */
    MYASSERT(close(fd) == 0);
}

void Checkpoint::handler(int signum)
{
    /* Access the X Window identifier from the SDL_Window struct */
    Display *display = getXDisplay();

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

        uint64_t last_request_read, request;

        if (display) {
#ifdef X_DPY_GET_LAST_REQUEST_READ
            last_request_read = X_DPY_GET_LAST_REQUEST_READ(display);
            request = X_DPY_GET_REQUEST(display);
#else
            last_request_read = static_cast<uint64_t>(display->last_request_read);
            request = static_cast<uint64_t>(display->request);
#endif
        // Save also dpy->xcb->last_flushed ?
        }

        readAllAreas();
        /* restoreInProgress was overwritten, putting the right value again */
        ThreadManager::restoreInProgress = true;

        /* Restoring the display values */
        if (display) {
#ifdef X_DPY_SET_LAST_REQUEST_READ
            X_DPY_SET_LAST_REQUEST_READ(display, last_request_read);
            X_DPY_SET_REQUEST(display, request);
#else
            display->last_request_read = static_cast<unsigned long>(last_request_read);
            display->request = static_cast<unsigned long>(request);
#endif
        }
    }
    else {
        writeAllAreas();
    }
    debuglogstdio(LCF_CHECKPOINT, "End restore.");
}

}
