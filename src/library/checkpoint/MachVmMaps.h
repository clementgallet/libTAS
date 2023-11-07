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

#ifndef LIBTAS_MACHVMMAPS_H
#define LIBTAS_MACHVMMAPS_H

#include "MemArea.h"

#include <mach/vm_map.h>

namespace libtas {
class MachVmMaps
{
    public:
        /* Connect a tesk to self */
        MachVmMaps();

        /* Parse the next memory section into the area */
        bool getNextArea(Area *area);

        /* Reset all internal variables */
        void reset();

    private:
        pid_t pid;
        mach_vm_address_t address;
        mach_vm_size_t size;
        natural_t depth;
        vm_region_submap_short_info_data_64_t info;
    
        struct dyld_all_image_infos *infos;

        const static int LIBRARYPATHSIZE = 1024;

        struct cache_library_data {
            uintptr_t addr;
            uint64_t size;
            vm_prot_t initprot;
            vm_prot_t maxprot;
            uint32_t flags;
            char path[LIBRARYPATHSIZE];
            
            cache_library_data &operator=(const cache_library_data& cld)
            {
                this->addr = cld.addr;
                this->size = cld.size;
                this->initprot = cld.initprot;
                this->maxprot = cld.maxprot;
                this->flags = cld.flags;
                strcpy(this->path, cld.path);
                return *this;
            }
            
            bool operator<(const cache_library_data& cld ) const
            {
                return (this->addr < cld.addr);
            }
        };
    
        cache_library_data* cache_libs;
        int cache_index;
};
}

#endif
