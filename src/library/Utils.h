/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_UTILS_H
#define LIBTAS_UTILS_H

#include <cstddef> // size_t
#include <cstdint> // uintptr_t
#include <unistd.h> // ssize_t

namespace libtas {
namespace Utils
{
    /* Fails or does entire write (returns count) */
    ssize_t writeAll(int fd, const void *buf, size_t count);
    
    /* Fails, succeeds, or partial read due to EOF (returns num read) 
     * return value:
     * -1: unrecoverable error
     * <n>: number of bytes read */
    ssize_t readAll(int fd, void *buf, size_t count);

    /* Returns the system page size */
    size_t getPageSize();

    /* Align address to page size */
    uintptr_t alignDownToPageSize(uintptr_t addr);
    uintptr_t alignUpToPageSize(uintptr_t addr);

    /* This function detects if the given page is a zero page or not. There is
     * scope of improving this function using some optimizations.
     *
     * TODO: One can use /proc/self/pagemap to detect if the page is backed by a
     * shared zero page.
     */
    bool isZeroPage(void *addr);
}
}

#endif
