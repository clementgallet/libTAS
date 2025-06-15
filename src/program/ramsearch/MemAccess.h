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
 */

#ifndef LIBTAS_MEMACCESS_H_INCLUDED
#define LIBTAS_MEMACCESS_H_INCLUDED

#include <stddef.h>
#include <sys/types.h>

/* Functions to read/write into game memroy */
namespace MemAccess {
    
    void init(pid_t pid, int addr_size);
    void fini();
    bool isInited();

    pid_t getPid();
    
    int getAddrSize();

    size_t read(void* local_addr, void* remote_addr, size_t size);
    size_t readAddr(void* local_addr, bool* valid);

    size_t write(void* local_addr, void* remote_addr, size_t size);    
}

#endif
