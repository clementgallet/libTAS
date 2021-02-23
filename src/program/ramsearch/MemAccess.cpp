/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#ifdef __unix__
#include <sys/uio.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>

static task_t task;
#endif

static pid_t game_pid;

void MemAccess::init(pid_t pid)
{
#if defined(__APPLE__) && defined(__MACH__)
    kern_return_t error = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        std::cerr << "task_for_pid() failed with message " << mach_error_string(kret) << std::endl;
        return;
    }
#endif

    game_pid = pid;
}

pid_t MemAccess::getPid()
{
    return game_pid;
}

int MemAccess::read(void* local_addr, void* remote_addr, size_t size)
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
    kern_return_t error = vm_read_overwrite(task, reinterpret_cast<vm_address_t>(remote_addr), size, local_addr, &ret_size);

    if (error != KERN_SUCCESS)
        return 0;
    return ret_size;
#endif
}

int MemAccess::write(void* local_addr, void* remote_addr, size_t size)
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
    size_t ret_size = size;
    kern_return_t error = vm_write(task, reinterpret_cast<vm_address_t>(remote_addr), local_addr, size);

    if (error != KERN_SUCCESS)
        return 0;
    return size;
#endif
}
