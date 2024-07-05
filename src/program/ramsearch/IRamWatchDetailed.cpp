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

#include "IRamWatchDetailed.h"
#include "MemSection.h"
#include "MemLayout.h"
#include "MemAccess.h"
#include "BaseAddresses.h"

#include "utils.h"

#include <sstream>
#include <fstream>
#include <iostream>

bool IRamWatchDetailed::isValid;

void IRamWatchDetailed::update_addr()
{
    isValid = true;
    if (isPointer) {
        /* Update the base address from the file and file offset */
        if (!base_address) {

            /* If file is empty, address is absolute */
            if (base_file.empty()) {
                base_address = base_file_offset;
            }
            else {
                base_address = BaseAddresses::getBaseAddress(base_file) + base_file_offset;
            }
        }
        
        pointer_addresses.assign(pointer_offsets.size(), 0);

        address = base_address;
        int i=0;
        for (auto offset : pointer_offsets) {
            uintptr_t next_address = MemAccess::readAddr(reinterpret_cast<void*>(address), &isValid);
            if (isValid)
                pointer_addresses[i++] = next_address;
            else
                return;

            address = next_address + offset;
        }
    }

}
