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

#ifndef LIBTAS_PROCMAPSAREA_H
#define LIBTAS_PROCMAPSAREA_H

#include <cstdint>
#include <sys/types.h>

// MTCP_PAGE_SIZE must be page-aligned:  multiple of sysconf(_SC_PAGESIZE).
// #define MTCP_PAGE_SIZE        4096
// #define MTCP_PAGE_MASK        (~(MTCP_PAGE_SIZE - 1))
// #define MTCP_PAGE_OFFSET_MASK (MTCP_PAGE_SIZE - 1)
// #define FILENAMESIZE          1024
//
#ifndef HIGHEST_VA

// If 32-bit process in 64-bit Linux, then Makefile overrides this address,
// with correct address for that case.
# ifdef __x86_64__

/* There's a segment, 7fbfffb000-7fc0000000 rw-p 7fbfffb000 00:00 0;
 * What is it?  It's busy (EBUSY) when we try to unmap it.
 */

// #  define HIGHEST_VA ((VA)0xFFFFFF8000000000)
// #  define HIGHEST_VA ((VA)0x8000000000)
#  define HIGHEST_VA ((VA)0x7f00000000)
# else // ifdef __x86_64__
#  define HIGHEST_VA ((VA)0xC0000000)
# endif // ifdef __x86_64__
#endif // ifndef HIGHEST_VA

// #define DELETED_FILE_SUFFIX " (deleted)"

#define FILENAMESIZE        1024

typedef char *VA;  /* VA = virtual address */

enum ProcMapsAreaProperties {
  // DMTCP_ZERO_PAGE = 0x0001,
  DMTCP_SKIP_WRITING_TEXT_SEGMENTS = 0x0002
};

union Area {
    struct {
        VA addr;   // args required for mmap to restore memory area
        VA endAddr;   // args required for mmap to restore memory area
        size_t size;
        off_t offset;
        int prot;
        int flags;
        unsigned int long devmajor;
        unsigned int long devminor;
        ino_t inodenum;
        uint64_t properties;
        char name[FILENAMESIZE];
    };
    char _padding[4096];
};

#endif
