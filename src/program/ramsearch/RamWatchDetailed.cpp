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
#include <cstring>

MemValueType RamWatchDetailed::get_value(bool& is_valid)
{
    MemValueType value;
    value.v_uint64_t = 0;

    is_valid = true;
    
    /* Update the actual address to look at (in case of pointer chain) */
    if (is_pointer) {
        /* Update the base address from the file and file offset */
        if (!base_address) {

            /* If file is empty, address is absolute */
            if (base_file.empty()) {
                base_address = base_file_offset;
            }
            else {
                base_address = BaseAddresses::getAddress(base_file, base_file_offset);
            }
        }
        
        pointer_addresses.assign(pointer_offsets.size(), 0);

        address = base_address;
        int i=0;
        for (auto offset : pointer_offsets) {
            uintptr_t next_address = MemAccess::readAddr(reinterpret_cast<void*>(address), &is_valid);
            if (is_valid)
                pointer_addresses[i++] = next_address;
            else
                return value;

            address = next_address + offset;
        }
    }
    
    if (value_type == RamType::RamArray) {
        is_valid = (MemAccess::read(value.v_array, reinterpret_cast<void*>(address), array_size) == (size_t)array_size);
        value.v_array[RAM_ARRAY_MAX_SIZE] = array_size;
    }
    else if (value_type == RamType::RamCString) {
        is_valid = (MemAccess::read(value.v_cstr, reinterpret_cast<void*>(address), RAM_ARRAY_MAX_SIZE) > 0); 
        value.v_cstr[RAM_ARRAY_MAX_SIZE] = 0;
    }
    else
        is_valid = (MemAccess::read(&value, reinterpret_cast<void*>(address), MemValue::type_size(value_type)) == (size_t)MemValue::type_size(value_type));
    return value;
}

const char* RamWatchDetailed::value_str()
{
    bool is_valid = true;
    MemValueType value = get_value(is_valid);
    if (!is_valid)
        return "??????";

    return MemValue::to_string(&value, value_type, hex);
}

int RamWatchDetailed::poke_value(const char* str_value)
{
    MemValueType value = MemValue::from_string(str_value, value_type, hex);
    return poke_value(value);
}

int RamWatchDetailed::poke_value(MemValueType value)
{
    /* Write value into the game process address */
    if (value_type == RamType::RamArray)
        return MemAccess::write(value.v_array, reinterpret_cast<void*>(address), value.v_array[RAM_ARRAY_MAX_SIZE]);
    else if (value_type == RamType::RamCString)
        return MemAccess::write(value.v_cstr, reinterpret_cast<void*>(address), strlen(value.v_cstr)+1);
    else
        return MemAccess::write(&value, reinterpret_cast<void*>(address), MemValue::type_size(value_type));
}

void RamWatchDetailed::keep_frozen()
{
    if (is_frozen) {
        poke_value(frozen_value);
    }
}
