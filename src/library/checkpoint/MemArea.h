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

#ifndef LIBTAS_MEMAREA_H
#define LIBTAS_MEMAREA_H

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

namespace libtas {
struct Area {
    enum PageFlag {
        NONE,
        NO_PAGE, /* No physical page is mapped at the virtual address */
        ZERO_PAGE, /* Entire page is zero */
        FULL_PAGE, /* Area contains a copy of the page */
        BASE_PAGE, /* Page was not modified from base savestate */
        COMPRESSED_PAGE, /* Full page but compressed */
        FILE_PAGE, /* Page is identical to the original mapped file */
    };

    void* addr;
    void* endAddr;
    size_t size;
    off_t offset;
    int prot;
    int max_prot;

    enum AreaFlag {
        AREA_ANON = 0x01, /* Anonymous mapping */
        AREA_FILE = 0x02, /* File mapping */
        AREA_PRIV = 0x04, /* Private mapping */
        AREA_SHARED = 0x08, /* Shared mapping */
        AREA_STACK = 0x10, /* Stack */
        AREA_HEAP = 0x20, /* Heap */
        AREA_MEMFD = 0x40, /* Region created by memfd_create() */
        AREA_SHM = 0x80, /* Region created by mapping a file inside /dev/shm/ */
        AREA_SAVEFILE = 0x100, /* Extra region created to store the content of savefiles */
    };
    int flags;
    unsigned int long devmajor;
    unsigned int long devminor;
    ino_t inodenum;
    bool skip;
    bool uncommitted;
    off_t page_offset; // position of the first area page in the pages file (in bytes)
    uint64_t hash;
    int fd; // file descriptor of the file mapped to this area

    enum {
        FILENAMESIZE = 1024
    };
    
    char name[FILENAMESIZE];
    
    /* for shared area, path to the corresponding file inside /proc/self/map_files
     * big enough to hold 64-bit addresses */
    char map_file[46];

    explicit operator bool() const {
        return (addr != nullptr);
    }
    
    /* Indicate if the area belongs to the standard mapping. Excludes savefiles */
    bool isStandard() const {return (addr != nullptr) && !(flags & AreaFlag::AREA_SAVEFILE);}

    void print(const char* prefix) const;

    int toMmapFlag() const;
    
    bool isSkipped() const;
    
    /* Returns if the area is guaranteed to be uncommitted based only on /proc/PID/maps values */
    bool isUncommitted(int spmfd) const;
    
    /* For deleted files, detect if a file descriptor of that file exists */
    void fillDeletedFd();
};
}

#endif
