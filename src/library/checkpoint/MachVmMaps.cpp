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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "MachVmMaps.h"
// #include "ReservedMemory.h"
#include "../logging.h"
// #include "../Utils.h"

// #include <fcntl.h>
#include <unistd.h>
// #include <sys/mman.h>
// #include <cstring>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <libproc.h>

namespace libtas {

MachVmMaps::MachVmMaps()
{
    task = MACH_PORT_NULL;
    pid = getpid();

    kern_return_t kret = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "task_for_pid() failed with error %d - %s", kret, mach_error_string(kret));
        return;
    }

    reset();
}

void MachVmMaps::reset()
{
    address = 0;
    size = 0;

    /* Read the first region */
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object_name;
    kern_return_t kret = mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t) &info, &count, &object_name);

    if (kret != KERN_SUCCESS) {
        debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "mach_vm_region: Error %d - %s", kret, mach_error_string(kret));
        return;
    }
}

bool MachVmMaps::getNextArea(Area *area)
{
    kern_return_t kret;

    mach_vm_address_t prev_address;
    vm_region_basic_info_data_t prev_info;
    mach_vm_size_t prev_size;
    mach_port_t object_name;
  
    memcpy (&prev_info, &info, sizeof (vm_region_basic_info_data_t));
    prev_address = address;
    prev_size = size;
 
    for (;;) {
        address = prev_address + prev_size;

        /* Check to see if address space has wrapped around. */
        if (address == 0) break;

        mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
        kret = mach_vm_region (task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t) &info, &count, &object_name);

        if (kret != KERN_SUCCESS) {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "mach_vm_region failed for address %p - Error: %x\n", address,(kret));
            size = 0;
            if (address >= 0x4000000) return false;
            break;
        }

        if (address != prev_address + prev_size)
            break;

        if ((info.protection != prev_info.protection)
        || (info.max_protection != prev_info.max_protection)
        || (info.inheritance != prev_info.inheritance)
        || (info.shared != prev_info.reserved)
        || (info.reserved != prev_info.reserved))
            break;

        prev_size += size;
    }
   
    area->addr = reinterpret_cast<void*>(prev_address);
    area->endAddr = reinterpret_cast<void*>(prev_address+prev_size);
    area->size = static_cast<size_t>(prev_size);

    area->prot = prev_info.protection;
    area->max_prot = prev_info.max_protection;
    area->flags = prev_info.shared ? 1 : 0; // TODO

    proc_regionfilename(pid, address, area->name, FILENAMESIZE);


    area->skip = false;

    return true;
}

}
