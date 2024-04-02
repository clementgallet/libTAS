/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "MemAccess.h"

#include <stdint.h>
#include <iostream>
#ifdef __unix__
#include <sys/uio.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/vm_map.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>

static task_t task;
#endif

static pid_t game_pid;
static int game_addr_size;

void MemAccess::init(pid_t pid, int addr_size)
{
#if defined(__APPLE__) && defined(__MACH__)
    kern_return_t error = task_for_pid(mach_task_self(), pid, &task);
    if (error != KERN_SUCCESS) {
        std::cerr << "task_for_pid() failed with message " << mach_error_string(error) << std::endl;
        return;
    }
#endif

    game_pid = pid;
    game_addr_size = addr_size;
}

pid_t MemAccess::getPid()
{
    return game_pid;
}

int MemAccess::getAddrSize()
{
    return game_addr_size;
}

size_t MemAccess::read(void* local_addr, void* remote_addr, size_t size)
{
    if (!game_pid)
        return 0;
        
#ifdef __unix__
    struct iovec local, remote;
    local.iov_base = local_addr;
    local.iov_len = size;
    remote.iov_base = remote_addr;
    remote.iov_len = size;

    return process_vm_readv(game_pid, &local, 1, &remote, 1, 0);
#elif defined(__APPLE__) && defined(__MACH__)
    size_t ret_size = size;
    kern_return_t error = vm_read_overwrite(task, reinterpret_cast<vm_address_t>(remote_addr), size, reinterpret_cast<vm_address_t>(local_addr), &ret_size);

    if (error != KERN_SUCCESS)
        return 0;
    return ret_size;
#endif
}

uintptr_t MemAccess::readAddr(void* remote_addr, bool* valid)
{
    if (game_addr_size == 4) {
        uint32_t value32;
        *valid = (read(&value32, remote_addr, sizeof(uint32_t)) == sizeof(uint32_t));
        return static_cast<uintptr_t>(value32);
    }

    uint64_t value64;
    *valid = (read(&value64, remote_addr, sizeof(uint64_t)) == sizeof(uint64_t));
    return static_cast<uintptr_t>(value64);
}

size_t MemAccess::write(void* local_addr, void* remote_addr, size_t size)
{
    if (!game_pid)
        return 0;

#ifdef __unix__
    struct iovec local, remote;
    local.iov_base = local_addr;
    local.iov_len = size;
    remote.iov_base = remote_addr;
    remote.iov_len = size;

    return process_vm_writev(game_pid, &local, 1, &remote, 1, 0);    
#elif defined(__APPLE__) && defined(__MACH__)
    kern_return_t error = vm_write(task, reinterpret_cast<vm_address_t>(remote_addr), reinterpret_cast<vm_offset_t>(local_addr), size);

    if (error != KERN_SUCCESS)
        return 0;
    return size;
#endif
}
