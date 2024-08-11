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

#include "RamWatchDetailed.h"
#include "MemSection.h"
#include "MemLayout.h"
#include "MemAccess.h"
#include "BaseAddresses.h"

#include "utils.h"

#include <sstream>
#include <fstream>
#include <iostream>

bool RamWatchDetailed::isValid;

void RamWatchDetailed::update_addr()
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

value_t RamWatchDetailed::get_value()
{
    update_addr();

    value_t value;
    value.v_uint64_t = 0;

    if (!isValid)
        return value;

    isValid = (MemAccess::read(&value, reinterpret_cast<void*>(address), MemValue::type_size(value_type)) == (size_t)MemValue::type_size(value_type));
    return value;
}

const char* RamWatchDetailed::value_str()
{
    value_t value = get_value();
    if (!isValid)
        return "??????";

    return MemValue::to_string(&value, value_type, hex);
}

int RamWatchDetailed::poke_value(const char* str_value)
{
    value_t value = MemValue::from_string(str_value, value_type, hex);

    /* Write value into the game process address */
    return MemAccess::write(&value, reinterpret_cast<void*>(address), MemValue::type_size(value_type));
}
