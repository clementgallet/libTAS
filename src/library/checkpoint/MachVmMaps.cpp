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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "MachVmMaps.h"
#include "ReservedMemory.h"

#include "logging.h"

#include <unistd.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <mach/mach_traps.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>
#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <libproc.h>
#include <sys/mman.h> // MAP_SHARED, etc.

namespace libtas {

MachVmMaps::MachVmMaps()
{
    pid = getpid();

    /* Get DYLD task infos */
    struct task_dyld_info dyld_info;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    kern_return_t ret;
    ret = task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&dyld_info, &count);
    if (ret != KERN_SUCCESS) {
        debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not get task info");
    }
    
    /* Get image array's size and address */
    infos = reinterpret_cast<struct dyld_all_image_infos*>(dyld_info.all_image_info_addr);

    /* Get the list of all __DATA segments of the loaded cache shared libraries, because they don't
     * appear properly when calling mach_vm_region_recurse(). The other segments
     * (__TEXT and __LINKEDIT) should not matter
     * Libraries are not listed by ascending address (because ASLR I presume?),
     * so we first store all relevent addresses, and sort them later. */
    
    /* Use our special reserved data because we must not allocate memory during savestate */
    cache_libs = static_cast<cache_library_data*>(ReservedMemory::getAddr(ReservedMemory::PSM_ADDR));
    cache_index = 0;

    uint32_t image_count = _dyld_image_count();
    for (int i = 0; i < image_count; ++i) {
//        debuglogstdio(LCF_CHECKPOINT, "   library %s at addr %p and slide %x", _dyld_get_image_name(i), _dyld_get_image_header(i), _dyld_get_image_vmaddr_slide(i));

        /* Shared cache starts at 0x7fff00000000. Other libraries will be processed normally */
        if (reinterpret_cast<uintptr_t>(_dyld_get_image_header(i)) < 0x7fff00000000)
            continue;
        
        const struct mach_header* header = _dyld_get_image_header(i);
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(_dyld_get_image_header(i));

        if (header->magic == MH_MAGIC_64) {
//            debuglogstdio(LCF_CHECKPOINT, "   64-bit header");
            const struct mach_header_64* header64 = reinterpret_cast<const struct mach_header_64*>(_dyld_get_image_header(i));
            ptr += sizeof(struct mach_header_64);
            
            for (int j=0; j<header64->ncmds; j++) {
                const struct load_command* cmd = reinterpret_cast<const struct load_command*>(ptr);
                if (cmd->cmd == LC_SEGMENT) {
                    const struct segment_command* segment = reinterpret_cast<const struct segment_command*>(ptr);
                    if (0 == strcmp("__DATA", segment->segname)) {
                        cache_libs[cache_index].addr = _dyld_get_image_vmaddr_slide(i) + segment->vmaddr;
                        /* vmsize is not rounded to the next page size, so we do the math here */
                        cache_libs[cache_index].size = ((segment->vmsize+4095)/4096)*4096;
                        cache_libs[cache_index].initprot = segment->initprot;
                        cache_libs[cache_index].maxprot = segment->maxprot;
                        cache_libs[cache_index].flags = segment->flags;
                        strncpy(cache_libs[cache_index].path, segment->segname, LIBRARYPATHSIZE);
                        cache_index++;
                        break;
                    }
                }
                if (cmd->cmd == LC_SEGMENT_64) {
                    const struct segment_command_64* segment = reinterpret_cast<const struct segment_command_64*>(ptr);
                    if (0 == strcmp("__DATA", segment->segname)) {
                        cache_libs[cache_index].addr = _dyld_get_image_vmaddr_slide(i) + segment->vmaddr;
                        /* vmsize is not rounded to the next page size, so we do the math here */
                        cache_libs[cache_index].size = ((segment->vmsize+4095)/4096)*4096;
                        cache_libs[cache_index].initprot = segment->initprot;
                        cache_libs[cache_index].maxprot = segment->maxprot;
                        cache_libs[cache_index].flags = segment->flags;
                        strncpy(cache_libs[cache_index].path, _dyld_get_image_name(i), LIBRARYPATHSIZE);
                        cache_index++;
                        break;
                    }
                }

                ptr += cmd->cmdsize;
            }
        }
        else {
            debuglogstdio(LCF_CHECKPOINT | LCF_ERROR, "   Unknown library magic: %x", header->magic);
        }
    }
    
    /* Add a last NULL element */
    cache_libs[cache_index].addr = 0;
    
    /* Sort all cache libraries */
    std::sort(&cache_libs[0], &cache_libs[cache_index]);

    reset();
}

void MachVmMaps::reset()
{
    address = 0;
    size = 0;
    depth = 0;
    cache_index = 0;
}

bool MachVmMaps::getNextArea(Area *area)
{
    kern_return_t kret;
    
    /* Are we getting a normal or cached library? */
    if (cache_index == 0) {
        /* Next section */
        address += size;
        
        while (true) {
            mach_msg_type_number_t count = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
            kret = mach_vm_region_recurse(mach_task_self(), &address, &size, &depth, reinterpret_cast<vm_region_recurse_info_t>(&info), &count);
            
            if (kret != KERN_SUCCESS) {
                address = 0;
                size = 0;
                return false;
            }
            if (!info.is_submap)
                break;
            
            depth++;
        }
    }
    
    /* Check if we must return a cache library */
    if (address >= 0x7fff00000000) {
        if (cache_libs[cache_index].addr == 0)
            return false;
        area->addr = reinterpret_cast<void*>(cache_libs[cache_index].addr);
        area->endAddr = reinterpret_cast<void*>(cache_libs[cache_index].addr + cache_libs[cache_index].size);
        area->size = cache_libs[cache_index].size;
        area->prot = cache_libs[cache_index].initprot;
        area->max_prot = cache_libs[cache_index].maxprot;
        strncpy(area->name, cache_libs[cache_index].path, LIBRARYPATHSIZE);
        area->skip = false;
        area->flags = Area::AREA_FILE | Area::AREA_PRIV;
        cache_index++;
        return true;
    }

    area->addr = reinterpret_cast<void*>(address);
    area->endAddr = reinterpret_cast<void*>(address+size);
    area->size = static_cast<size_t>(size);
    area->prot = info.protection;
    area->max_prot = info.max_protection;
    /* TODO: Implement own flags */
    switch (info.share_mode) {
        case SM_COW:
        case SM_PRIVATE:
        case SM_PRIVATE_ALIASED:
        case SM_LARGE_PAGE:
            area->flags = Area::AREA_PRIV;
            break;
        case SM_SHARED:
        case SM_TRUESHARED:
        case SM_SHARED_ALIASED:
            area->flags = Area::AREA_SHARED;
            break;
        case SM_EMPTY: // TODO
            area->flags = Area::AREA_PRIV;
            break;
        default:
            area->flags = Area::AREA_PRIV;
            break;
    }
    area->skip = false;

    area->name[0] = '\0';

    /* TODO: Process user tag */
    switch (info.user_tag) {
        case 0:
            /* No user tag usually means standard mapped file */
            errno = 0;
            kret = proc_regionfilename(pid, address, area->name, FILENAMESIZE);
            if ((kret < 0) || (errno != 0))
                area->name[0] = '\0';
            area->flags |= Area::AREA_FILE;
            break;
        case VM_MEMORY_MALLOC:
        case VM_MEMORY_MALLOC_SMALL:
        case VM_MEMORY_MALLOC_LARGE:
        case VM_MEMORY_MALLOC_HUGE:
        case VM_MEMORY_REALLOC:
        case VM_MEMORY_MALLOC_TINY:
        case VM_MEMORY_MALLOC_LARGE_REUSABLE:
        case VM_MEMORY_MALLOC_LARGE_REUSED:
        case VM_MEMORY_MALLOC_NANO:
            area->flags |= Area::AREA_ANON;
            break;
        case VM_MEMORY_SBRK:
            area->flags |= Area::AREA_HEAP;
            area->flags |= Area::AREA_ANON;
            break;
        case VM_MEMORY_STACK:
            area->flags |= Area::AREA_STACK;
            area->flags |= Area::AREA_ANON;
            break;
        case VM_MEMORY_GUARD:
            area->skip = true;
            area->flags |= Area::AREA_ANON;
            break;
        case VM_MEMORY_DYLIB:
            area->flags |= Area::AREA_FILE;
            break;
        default:
            area->flags |= Area::AREA_ANON;
            break;
    }
    
    return true;
}

}
