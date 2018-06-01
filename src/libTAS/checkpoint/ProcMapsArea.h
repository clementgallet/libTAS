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

#ifndef LIBTAS_PROCMAPSAREA_H
#define LIBTAS_PROCMAPSAREA_H

#include <cstdint>
#include <sys/types.h>

// If 32-bit process in 64-bit Linux, then Makefile overrides this address,
// with correct address for that case.
# ifdef __x86_64__

/* There's a segment, 7fbfffb000-7fc0000000 rw-p 7fbfffb000 00:00 0;
 * What is it?  It's busy (EBUSY) when we try to unmap it.
 */

// #  define HIGHEST_VA ((VA)0xFFFFFF8000000000)
// #  define HIGHEST_VA ((VA)0x8000000000)
#  define HIGHEST_VA ((void*)0x7f00000000)
# else // ifdef __x86_64__
#  define HIGHEST_VA ((void*)0xC0000000)
# endif // ifdef __x86_64__

// #define DELETED_FILE_SUFFIX " (deleted)"

#define FILENAMESIZE        1024

namespace libtas {
struct Area {
    enum ProcMapsAreaProperties {
        NONE = 0x00,
        PAGES = 0x01, /* Area is followed by property of every page */
        ZERO_PAGE = 0x02, /* Entire area or page is zero */
        SKIP = 0x04 /* Area is skipped */
    };

    void* addr;
    void* endAddr;
    size_t size;
    off_t offset;
    int prot;
    int flags;
    unsigned int long devmajor;
    unsigned int long devminor;
    ino_t inodenum;
    int properties;
    off_t page_offset; // position of the first area page in the pages file (in bytes)
    char name[FILENAMESIZE];

    void print(const char* prefix) const;
};
}

#endif
