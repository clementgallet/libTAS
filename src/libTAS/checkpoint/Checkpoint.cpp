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
#include "../logging.h"
#include "ProcSelfMaps.h"
#include "Utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define SAVESTATEPATH "/tmp/savestate"

void Checkpoint::writeAllAreas()
{
    int fd = open(SAVESTATEPATH, O_WRONLY | O_TRUNC);
    Area area;

    // DeviceInfo dev_info;
    // int stack_was_seen = 0;

    // if (getenv(ENV_VAR_SKIP_WRITING_TEXT_SEGMENTS) != NULL) {
    //     skipWritingTextSegments = true;
    // }

    debuglog(LCF_CHECKPOINT, "Performing checkpoint.");

    /* Finally comes the memory contents */
    ProcSelfMaps* procSelfMaps = new ProcSelfMaps();
    while (procSelfMaps->getNextArea(&area)) {

        // if ((uint64_t)area.addr == ProcessInfo::instance().restoreBufAddr()) {
        //     JASSERT(area.size == ProcessInfo::instance().restoreBufLen())
        //     ((void *)area.addr)
        //     (area.size)
        //     (ProcessInfo::instance().restoreBufLen());
        //     continue;
        // } else if (SharedData::isSharedDataRegion(area.addr)) {
        //     continue;
        // }

        /* Original comment:  Skip anything in kernel address space ---
        *   beats me what's at FFFFE000..FFFFFFFF - we can't even read it;
        * Added: That's the vdso section for earlier Linux 2.6 kernels.  For later
        *  2.6 kernels, vdso occurs at an earlier address.  If it's unreadable,
        *  then we simply won't copy it.  But let's try to read all areas, anyway.
        * **COMMENTED OUT:** if (area.addr >= HIGHEST_VA) continue;
        */

        /* If it's readable, but it's VDSO, it will be dangerous to restore it.
        * In 32-bit mode later Red Hat RHEL Linux 2.6.9 releases use 0xffffe000,
        * the last page of virtual memory.  Note 0xffffe000 >= HIGHEST_VA
        * implies we're in 32-bit mode.
        */
        if (area.addr >= HIGHEST_VA && area.addr == (VA)0xffffe000) {
            continue;
        }
        #ifdef __x86_64__

        /* And in 64-bit mode later Red Hat RHEL Linux 2.6.9 releases
        * use 0xffffffffff600000 for VDSO.
        */
        if (area.addr >= HIGHEST_VA && area.addr == (VA)0xffffffffff600000) {
            continue;
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

        if (!((area.prot & PROT_READ) || (area.prot & PROT_WRITE)) &&
        (area.name[0] != '\0') &&
        strcmp(area.name, "[heap]") &&
        strncmp(area.name, "[stack", 6)) {
            continue;
        }

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
        // } else if (Util::strEndsWith(area.name, DELETED_FILE_SUFFIX)) {
        //     /* Deleted File */
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

        if ((area.flags & MAP_PRIVATE) /*&& (area.prot & PROT_WRITE)*/) {
            area.flags |= MAP_ANONYMOUS;
        }

        /* Only write this image if it is not CS_RESTOREIMAGE.
        * Skip any mapping for this image - it got saved as CS_RESTOREIMAGE
        * at the beginning.
        */

        // the whole thing comes after the restore image
        writeAnArea(fd, &area);
    }

    // Release the memory.
    delete procSelfMaps;
    procSelfMaps = NULL;

    /* It's now safe to do this, since we're done using writememoryarea() */
    // remap_nscd_areas(*nscdAreas);

    area.addr = NULL; // End of data
    area.size = -1; // End of data
    write(fd, &area, sizeof(area));

    /* That's all folks */
    MYASSERT(close(fd) == 0);
}

void Checkpoint::writeAnArea(int fd, Area *area)
{
    void *addr = area->addr;

    if (!(area->flags & MAP_ANONYMOUS)) {
        debuglog(LCF_CHECKPOINT, "Save region ", addr, " (", area->name, ") with size ", area->size);
    } else if (area->name[0] == '\0') {
        debuglog(LCF_CHECKPOINT, "Save anonymous ", addr, " with size ", area->size);
    } else {
        debuglog(LCF_CHECKPOINT, "Save anonymous ", addr, " (", area->name, ") with size ", area->size);
    }

    if ((area->name[0]) == '\0') {
        char *brk = (char *)sbrk(0);
        if (brk > area->addr && brk <= area->addr + area->size) {
            strcpy(area->name, "[heap]");
        }
    }

    if (area->size == 0) {
        /* Kernel won't let us munmap this.  But we don't need to restore it. */
        debuglog(LCF_CHECKPOINT, "skipping over empty segment");
    } else if (0 == strcmp(area->name, "[vsyscall]") ||
    0 == strcmp(area->name, "[vectors]") ||
    0 == strcmp(area->name, "[vvar]") ||
    0 == strcmp(area->name, "[vdso]")) {
        debuglog(LCF_CHECKPOINT, "Skipping over memory special section");
    } else if ((area->prot & PROT_READ) == 0) {
        MYASSERT(mprotect(area->addr, area->size, area->prot | PROT_READ) == 0)

        Utils::writeAll(fd, area, sizeof(*area));
        Utils::writeAll(fd, area->addr, area->size);

        MYASSERT(mprotect(area->addr, area->size, area->prot) == 0)
    } else {
        /* Anonymous sections need to have their data copied to the file,
        *   as there is no file that contains their data
        * We also save shared files to checkpoint file to handle shared memory
        *   implemented with backing files
        */
        // JASSERT((area->flags & MAP_ANONYMOUS) || (area->flags & MAP_SHARED));
        bool skipWritingTextSegments = true;
        if (skipWritingTextSegments && (area->prot & PROT_EXEC)) {
            area->properties |= DMTCP_SKIP_WRITING_TEXT_SEGMENTS;
            Utils::writeAll(fd, area, sizeof(*area));
            debuglog(LCF_CHECKPOINT, "Skipping over text segments");
        } else {
            Utils::writeAll(fd, area, sizeof(*area));
            Utils::writeAll(fd, area->addr, area->size);
        }
    }
}
