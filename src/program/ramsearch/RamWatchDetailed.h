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

#ifndef LIBTAS_RAMWATCHDETAILED_H_INCLUDED
#define LIBTAS_RAMWATCHDETAILED_H_INCLUDED

#include <string>
#include <vector>
#include <cstdint>

#include "MemValue.h"

class RamWatchDetailed {
public:
    RamWatchDetailed(uintptr_t addr, int type) : value_type(type), address(addr) {};

    /* Return the current value of the ram watch as a string */
    const char* value_str();

    /* Poke a value (given as a string) into the ram watch address. Return
     * the result of process_vm_writev call */
    int poke_value(const char* str_value);

    /* If value is frozen, poke the value into ram watch */
    void keep_frozen();

    int value_type;
    uintptr_t address;
    std::string label;
    bool hex;
    int array_size;

    bool is_pointer;
    std::vector<int> pointer_offsets;
    /* Intermediate addresses, only for indication */
    std::vector<uintptr_t> pointer_addresses;
    uintptr_t base_address;
    off_t base_file_offset;
    std::string base_file;

    bool is_frozen;
    MemValueType frozen_value;

private:
    /* Return the current value of the ram watch as a MemValueType */
    MemValueType get_value(bool& is_valid);

    /* Poke a value (given as a MemValueType) into the ram watch address. Return
     * the result of process_vm_writev call */
    int poke_value(MemValueType value);

};

#endif
