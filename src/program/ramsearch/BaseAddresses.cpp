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

#include "BaseAddresses.h"
#include "MemLayout.h"
#include "MemAccess.h"
#include "../utils.h"

#include <map>
#include <utility>
#include <memory>
#include <iostream>


// #include <stdint.h>
// #include <iostream>
// #ifdef __unix__
// #include <sys/uio.h>
// #elif defined(__APPLE__) && defined(__MACH__)
// #include <mach/vm_map.h>
// #include <mach/mach_traps.h>
// #include <mach/mach_init.h>
// #include <mach/mach_error.h>
// 
// static task_t task;
// #endif

static std::map<std::string,std::pair<uintptr_t, uintptr_t>> library_addresses;

void BaseAddresses::load()
{
    library_addresses.clear();
    
    std::unique_ptr<MemLayout> memlayout (new MemLayout(MemAccess::getPid()));
    
    MemSection section;
    std::string previous_file = ""; // file of previous section
    bool previous_stored = false; // was the last section stored?
    while (memlayout->nextSection(MemSection::MemAll, 0, section)) {
        std::string file = fileFromPath(section.filename);
        
        /* Special case for BSS, which belongs to executable memory */
        if (section.type == MemSection::MemBSS) {
            library_addresses[previous_file].second = section.endaddr;
            previous_stored = true;
            continue;
        }
        
        if (file.empty()) {
            previous_file = "";
            previous_stored = false;
            continue;
        }

        /* Check if new file */
        if (previous_file.empty() || (file.compare(previous_file) != 0)) {
            library_addresses[file] = std::make_pair(section.addr, section.endaddr);
            previous_stored = true;
        }
        else {
            /* If the file is the same as previous one, append the addresses
             * if we are interested in the file */
            if (previous_stored) {
                library_addresses[file].second = section.endaddr;
            }            
        }

        previous_file = file;
    }
}

uintptr_t BaseAddresses::findNewFile(std::string file)
{
    std::unique_ptr<MemLayout> memlayout (new MemLayout(MemAccess::getPid()));
    
    MemSection section;
    uintptr_t base_addr = 0;
    while (memlayout->nextSection(MemSection::MemAll, 0, section)) {
        std::string section_file = fileFromPath(section.filename);
        
        /* Only add the requested file */
        if (section_file.compare(file) == 0) {
            if (section.execflag) {
                base_addr = section.addr;
                library_addresses[file] = std::make_pair(section.addr, section.endaddr);
            }
            else {
                library_addresses[file].second = section.endaddr;
                /* Library mapping should end with RW section. Return here to
                 * not load the remaining layout */
                if (section.type == MemSection::MemFileMappingRW)
                    return base_addr;
            }
        }
    }
    
    return 0;
}

uintptr_t BaseAddresses::getBaseAddress(std::string file)
{
    if (library_addresses.empty())
        load();

    auto it = library_addresses.find(file);
    if (it != library_addresses.end()) {
        return it->second.first;        
    }
    else {
        return findNewFile(file);
    }
}

std::string BaseAddresses::getFileAndOffset(uintptr_t addr, off_t &offset)
{
    if (library_addresses.empty())
        load();

    for (const auto& it : library_addresses) {
        if ((addr >= it.second.first) && (addr < it.second.second)) {
            /* Found a matching section */        
            /* For stack, save the negative offset from the end */
            if (it.first.find("[stack") == 0) {
                offset = addr - it.second.second;
            }
            else {
                offset = addr - it.second.first;                
            }
            return it.first;
        }
    }
    offset = 0;
    return "";
}
