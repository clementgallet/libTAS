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
#include "ProcSelfMaps.h"
#include "Utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <X11/Xlibint.h>
// #define XLIB_ILLEGAL_ACCESS
#include "../sdlwindows.h"

#define SAVESTATEPATH "/tmp/savestate"

#define MB 1024 * 1024
#define RESTORE_TOTAL_SIZE 5 * MB

namespace Checkpoint {

static intptr_t restoreAddr = 0;
static size_t restoreLength = 0;

void init()
{
    /* Create a special place to hold restore memory.
     * will be used for the second stack we will switch to, as well as
     * the ProcSelfMaps object that need some space.
     */
    if (restoreAddr == 0) {
        restoreLength = RESTORE_TOTAL_SIZE;
        void* addr = mmap(nullptr, restoreLength + (2 * 4096), PROT_NONE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        MYASSERT(addr != MAP_FAILED)
        restoreAddr = reinterpret_cast<intptr_t>(addr) + 4096;
        MYASSERT(mprotect(reinterpret_cast<void*>(restoreAddr), restoreLength, PROT_READ | PROT_WRITE) == 0)
    }

    /* Setup an alternate signal stack using the above allocated memory */
    stack_t ss;
    ss.ss_sp = reinterpret_cast<void*>(restoreAddr + MB);
    ss.ss_size = RESTORE_TOTAL_SIZE - MB;
    ss.ss_flags = 0;
    MYASSERT(sigaltstack(&ss, nullptr) == 0)

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

void handler(int signum)
{

    if (ThreadManager::restoreInProgress) {
        /* Before reading from the savestate, we must keep some values from
         * the connection to the X server, because they are used for checking
         * consistency of requests. We can store them in this (alternate) stack
         * which will be preserved.
         */

        /* Access the X Window identifier from the SDL_Window struct */
        Display *display = getXDisplay();
        uint64_t last_request_read, request;

        if (display) {
#ifdef X_DPY_GET_LAST_REQUEST_READ
            last_request_read = X_DPY_GET_LAST_REQUEST_READ(display);
            request = X_DPY_GET_REQUEST(display);
#else
            last_request_read = static_cast<uint64_t>(display->last_request_read);
            request = static_cast<uint64_t>(display->request);
#endif
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
}

static bool skipArea(Area *area)
{
    /* If it's readable, but it's VDSO, it will be dangerous to restore it.
    * In 32-bit mode later Red Hat RHEL Linux 2.6.9 releases use 0xffffe000,
    * the last page of virtual memory.  Note 0xffffe000 >= HIGHEST_VA
    * implies we're in 32-bit mode.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (VA)0xffffe000) {
        return true;
    }
    #ifdef __x86_64__

    /* And in 64-bit mode later Red Hat RHEL Linux 2.6.9 releases
    * use 0xffffffffff600000 for VDSO.
    */
    if (area->addr >= HIGHEST_VA && area->addr == (VA)0xffffffffff600000) {
        return true;
    }
    #endif // ifdef __x86_64__

    /* Skip anything that has no read or execute permission.  This occurs
    * on one page in a Linux 2.6.9 installation.  No idea why.  This code
    * would also take care of kernel sections since we don't have read/execute
    * permission there.
    *
    * EDIT: We should only skip the "---p" section for the shared libraries.
    * Anonymous memory areas with no rwx permission should be saved regardless
    * as the process might have removed the permissions temporarily and might
    * want to use it later.
    *
    * This happens, for example, with libpthread where the pthread library
    * tries to recycle thread stacks. When a thread exits, libpthread will
    * remove the access permissions from the thread stack and later, when a
    * new thread is created, it will provide the proper permission to this
    * area and use it as the thread stack.
    *
    * If we do not restore this area on restart, the area might be returned by
    * some mmap() call. Later on, when pthread wants to use this area, it will
    * just try to use this area which now belongs to some other object. Even
    * worse, the other object can then call munmap() on that area after
    * libpthread started using it as thread stack causing the parts of thread
    * stack getting munmap()'d from the memory resulting in a SIGSEGV.
    *
    * We suspect that libpthread is using mmap() instead of mprotect to change
    * the permission from "---p" to "rw-p".
    *
    * Also, on SUSE 12, if this region was part of heap, the protected region
    * may have the label "[heap]".  So, we also save the memory region if it
    * has label "[heap]", "[stack]", or  "[stack:XXX]".
    */

    if (!((area->prot & PROT_READ) || (area->prot & PROT_WRITE)) &&
    (area->name[0] != '\0') &&
    strcmp(area->name, "[heap]") &&
    strncmp(area->name, "[stack", 6)) {
        return true;
    }

    if (strstr(area->name, "(deleted)")) {
        /* Deleted File */
        return true;
    }

    // if (strstr(area->name, "i965_dri")) { // libX11.so
    //     return true;
    // }

    if (area->size == 0) {
        /* Kernel won't let us munmap this.  But we don't need to restore it. */
        // debuglogstdio(LCF_CHECKPOINT, "skipping over empty segment");
        return true;
    }

    if (0 == strcmp(area->name, "[vsyscall]") ||
    0 == strcmp(area->name, "[vectors]") ||
    0 == strcmp(area->name, "[vvar]") ||
    0 == strcmp(area->name, "[vdso]")) {
        // debuglogstdio(LCF_CHECKPOINT, "Skipping over memory special section");
        return true;
    }

    if (area->prot & PROT_EXEC) {
        // area->properties |= DMTCP_SKIP_WRITING_TEXT_SEGMENTS;
        // Utils::writeAll(fd, area, sizeof(*area));
        // debuglogstdio(LCF_CHECKPOINT, "Skipping over text segments");
        return true;
    }

    if ((area->addr == reinterpret_cast<void*>(restoreAddr)) && (area->size == restoreLength)) {
        // debuglogstdio(LCF_CHECKPOINT, "Skipping over our reserved section");
        return true;
    }

    /* Right now, skip if we don't have write permission */
    if (!(area->prot & PROT_WRITE)) {
        // debuglogstdio(LCF_CHECKPOINT, "Skipping over no write permission");
        return true;
    }

    /* Right now, skip for shared memory */
    if (area->flags & MAP_SHARED) {
        // debuglogstdio(LCF_CHECKPOINT, "Skipping over shared memory");
        return true;
    }

    return false;
}

void writeAllAreas()
{
    Area area;

    // DeviceInfo dev_info;
    // int stack_was_seen = 0;

    // if (getenv(ENV_VAR_SKIP_WRITING_TEXT_SEGMENTS) != NULL) {
    //     skipWritingTextSegments = true;
    // }

    debuglogstdio(LCF_CHECKPOINT, "Performing checkpoint.");

    /* Finally comes the memory contents */
    ProcSelfMaps procSelfMaps(restoreAddr, MB);

    int fd = creat(SAVESTATEPATH, 0644);
    MYASSERT(fd != -1)

    while (procSelfMaps.getNextArea(&area)) {

        // if ((uint64_t)area.addr == ProcessInfo::instance().restoreBufAddr()) {
        //     JASSERT(area.size == ProcessInfo::instance().restoreBufLen())
        //     ((void *)area.addr)
        //     (area.size)
        //     (ProcessInfo::instance().restoreBufLen());
        //     continue;
        // } else if (SharedData::isSharedDataRegion(area.addr)) {
        //     continue;
        // }



        // if (Util::strStartsWith(area.name, DEV_ZERO_DELETED_STR) ||
        // Util::strStartsWith(area.name, DEV_NULL_DELETED_STR)) {
        //     /* If the process has an area labeled as "/dev/zero (deleted)", we mark
        //     *   the area as Anonymous and save the contents to the ckpt image file.
        //     * If this area has a MAP_SHARED attribute, it should be replaced with
        //     *   MAP_PRIVATE and we won't do any harm because, the /dev/zero file is
        //     *   an absolute source and sink. Anything written to it will be
        //     *   discarded and anything read from it will be all zeros.
        //     * The following call to mmap will create "/dev/zero (deleted)" area
        //     *         mmap(addr, size, protection, MAP_SHARED | MAP_ANONYMOUS, 0, 0)
        //     *
        //     * The above explanation also applies to "/dev/null (deleted)"
        //     */
        //     JTRACE("saving area as Anonymous") (area.name);
        //     area.flags = MAP_PRIVATE | MAP_ANONYMOUS;
        //     area.name[0] = '\0';
        // } else if (Util::isSysVShmArea(area)) {
        //     JTRACE("saving area as Anonymous") (area.name);
        //     area.flags = MAP_PRIVATE | MAP_ANONYMOUS;
        //     area.name[0] = '\0';
        // } else if (Util::isNscdArea(area)) {
        //     /* Special Case Handling: nscd is enabled*/
        //     area.prot = PROT_READ | PROT_WRITE;
        //     area.properties |= DMTCP_ZERO_PAGE;
        //     area.flags = MAP_PRIVATE | MAP_ANONYMOUS;
        //     Util::writeAll(fd, &area, sizeof(area));
        //     continue;
        // } else if (Util::isIBShmArea(area)) {
        //     // TODO: Don't checkpoint infiniband shared area for now.
        //     continue;
        // } else if (area.name[0] == '/' && strstr(&area.name[1], "/") != NULL) {
        /* If an absolute pathname
        * Posix and SysV shared memory segments can be mapped as /XYZ
        */
        // }

        /* Force the anonymous flag if it's a private writeable section, as the
        * data has probably changed from the contents of the original images.
        */

        /* We also do this for read-only private sections as it's possible
        * to modify a page there, too (via mprotect).
        */


        if (skipArea(&area)) {
            area.properties |= SKIP;
            Utils::writeAll(fd, &area, sizeof(area));
            continue;
        }

        if ((area.flags & MAP_PRIVATE) /*&& (area.prot & PROT_WRITE)*/) {
            area.flags |= MAP_ANONYMOUS;
        }

        Utils::writeAll(fd, &area, sizeof(area));

        /* Only write this image if it is not CS_RESTOREIMAGE.
        * Skip any mapping for this image - it got saved as CS_RESTOREIMAGE
        * at the beginning.
        */

        // the whole thing comes after the restore image
        writeAnArea(fd, &area);
    }

    area.addr = nullptr; // End of data
    area.size = 0; // End of data
    write(fd, &area, sizeof(area));

    /* That's all folks */
    MYASSERT(close(fd) == 0);
}

void writeAnArea(int fd, Area *area)
{
    void *addr = area->addr;

    if (!(area->flags & MAP_ANONYMOUS)) {
        debuglogstdio(LCF_CHECKPOINT, "Save region %p (%s) with size %d", addr, area->name, area->size);
    } else if (area->name[0] == '\0') {
        debuglogstdio(LCF_CHECKPOINT, "Save anonymous %p with size %d", addr, area->size);
    } else {
        debuglogstdio(LCF_CHECKPOINT, "Save anonymous %p (%s) with size %d", addr, area->name, area->size);
    }

    // if ((area->name[0]) == '\0') {
    //     char *brk = (char *)sbrk(0);
    //     if (brk > area->addr && brk <= area->addr + area->size) {
    //         strcpy(area->name, "[heap]");
    //     }
    // }


    if ((area->prot & PROT_READ) == 0) {
        MYASSERT(mprotect(area->addr, area->size, area->prot | PROT_READ) == 0)
    }
        Utils::writeAll(fd, area->addr, area->size);

    if ((area->prot & PROT_READ) == 0) {
        MYASSERT(mprotect(area->addr, area->size, area->prot) == 0)
    }
}

void readAllAreas()
{
    int fd = open(SAVESTATEPATH, O_RDONLY);
    Area current_area;
    Area saved_area;

    debuglogstdio(LCF_CHECKPOINT, "Performing restore.");

    /* Read the memory mapping */
    ProcSelfMaps procSelfMaps(restoreAddr, MB);

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
}

int readAndCompAreas(int fd, Area *saved_area, Area *current_area)
{
    if (saved_area->addr != nullptr && !(saved_area->properties & SKIP)) {
        if (!(saved_area->flags & MAP_ANONYMOUS)) {
            debuglogstdio(LCF_CHECKPOINT, "Restore region %p (%s) with size %d", (void*)saved_area->addr, saved_area->name, saved_area->size);
        } else if (saved_area->name[0] == '\0') {
            debuglogstdio(LCF_CHECKPOINT, "Restore anonymous %p with size %d", saved_area->addr, saved_area->size);
        } else {
            debuglogstdio(LCF_CHECKPOINT, "Restore anonymous %p (%s) with size %d", saved_area->addr, saved_area->name, saved_area->size);
        }
    }

    /* Are areas overlapping? */
    if ((saved_area->addr != nullptr) && (current_area->addr != nullptr) &&
        (((saved_area->addr >= current_area->addr) && (saved_area->addr < current_area->endAddr)) ||
        ((current_area->addr >= saved_area->addr) && (current_area->addr < saved_area->endAddr)))) {

        /* Check if it is a skipped area */
        if (saved_area->properties & SKIP) {
            // debuglogstdio(LCF_CHECKPOINT, "Section is skipped");
            if (!skipArea(current_area)) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Current section is skipped anymore !?");
            }
            return 0;
        }

        /* Check if we need resizing */
        if ((saved_area->addr != current_area->addr) || (saved_area->endAddr != current_area->endAddr)) {

            /* Try to resize the area using mremap */
            void *newAddr;
            if (saved_area->addr == current_area->addr) {
                debuglogstdio(LCF_CHECKPOINT, "Changing size from %d to %d", current_area->size, saved_area->size);
                newAddr = mremap(current_area->addr, current_area->size, saved_area->size, 0);
            }
            else {
                debuglogstdio(LCF_CHECKPOINT, "Changing size and location from %p to %p", current_area->addr, saved_area->addr);
                newAddr = mremap(current_area->addr, current_area->size, saved_area->size, MREMAP_FIXED, saved_area->addr);
            }

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
        }

        /* Now copy the data */
        if (!(current_area->prot & PROT_WRITE)) {
            debuglogstdio(LCF_CHECKPOINT, "Current area lost its write permission");
            MYASSERT(mprotect(saved_area->addr, saved_area->size, current_area->prot | PROT_WRITE) == 0)
        }

        debuglogstdio(LCF_CHECKPOINT, "Writing to memory!");
        Utils::readAll(fd, saved_area->addr, saved_area->size);

        if (!(current_area->prot & PROT_WRITE)) {
            debuglogstdio(LCF_CHECKPOINT, "Recover permission");
            MYASSERT(mprotect(saved_area->addr, saved_area->size, current_area->prot) == 0)
        }

        return 0;
    }

    /* Areas are not overlapping */
    if ((saved_area->addr == nullptr) || (saved_area->addr > current_area->addr)) {

        /* This current area must be deallocated */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be deallocated", current_area->addr, current_area->name, current_area->size);
        MYASSERT(munmap(current_area->addr, current_area->size) == 0)
        return 1;
    }

    if ((current_area->addr == nullptr) || (saved_area->addr < current_area->addr)) {

        if (saved_area->properties & SKIP) {
            // debuglogstdio(LCF_CHECKPOINT, "Section is skipped");
            return -1;
        }

        /* This saved area must be allocated */
        debuglogstdio(LCF_CHECKPOINT, "Region %p (%s) with size %d must be allocated", saved_area->addr, saved_area->name, saved_area->size);

        if (saved_area->flags & MAP_ANONYMOUS) {

            int imagefd = -1;
            if (saved_area->name[0] == '/') { /* If not null string, not [stack] or [vdso] */
                imagefd = open(saved_area->name, O_RDONLY, 0);
                if (imagefd >= 0) {
                    /* If the current file size is smaller than the original, we map the region
                    * as private anonymous. Note that with this we lose the name of the region
                    * but most applications may not care.
                    */
                    off_t curr_size = lseek(imagefd, 0, SEEK_END);
                    MYASSERT(curr_size != -1);
                    if (static_cast<size_t>(curr_size) < saved_area->offset + saved_area->size) {
                        close(imagefd);
                        imagefd = -1;
                        saved_area->offset = 0;
                    } else {
                        saved_area->flags ^= MAP_ANONYMOUS;
                    }
                }
            }

            if (saved_area->flags & MAP_ANONYMOUS) {
                debuglogstdio(LCF_CHECKPOINT, "Restoring anonymous area, %d bytes at %p", saved_area->size, saved_area->addr);
            } else {
                debuglogstdio(LCF_CHECKPOINT, "Restoring to non-anonymous area from anonymous area, %d bytes at %p from %s + %d", saved_area->size, saved_area->addr, saved_area->name, saved_area->offset);
            }

            /* Create the memory area */

            void *mmappedat = mmap(saved_area->addr, saved_area->size, saved_area->prot | PROT_WRITE,
                saved_area->flags, imagefd, saved_area->offset);

            if (mmappedat == MAP_FAILED) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Mapping %d bytes at %p failed", saved_area->size, saved_area->addr);
                lseek(fd, saved_area->size, SEEK_CUR);
                return -1;
            }
            if (mmappedat != saved_area->addr) {
                debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "Area at %p got mmapped to %p", saved_area->addr, mmappedat);
                lseek(fd, saved_area->size, SEEK_CUR);
                return -1;
            }

            /* Close image file (fd only gets in the way) */
            if (imagefd >= 0) {
                close(imagefd);
            }

            Utils::readAll(fd, saved_area->addr, saved_area->size);
            if (!(saved_area->prot & PROT_WRITE)) {
                MYASSERT(mprotect(saved_area->addr, saved_area->size, saved_area->prot) == 0)
            }
        }
        return -1;
    }

    MYASSERT(false)
    return 0;
}

}
