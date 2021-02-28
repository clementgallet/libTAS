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

#ifndef LIBTAS_RAMWATCH_H_INCLUDED
#define LIBTAS_RAMWATCH_H_INCLUDED

#include "CompareEnums.h"
#include <cstdint>
#include <sys/types.h>

class RamWatch {
public:
    uintptr_t address;
    uint64_t previous_value;

    static bool isValid;
    static int type;
    static size_t type_size;

    enum RamType {
        RamUnsignedChar,
        RamChar,
        RamUnsignedShort,
        RamShort,
        RamUnsignedInt,
        RamInt,
        RamUnsignedLong,
        RamLong,
        RamFloat,
        RamDouble,
    };

    RamWatch(uintptr_t addr) : address(addr) {};

    /* Format number into a string */
    const char* tostring(bool hex, uint64_t value) const;

    /* Get the current value */
    uint64_t get_value() const;

    /* Store the current value and returns if succeeded */
    bool query();

    /* Compare function */
    bool check(uint64_t value, CompareType compare_type, CompareOperator compare_operator, double compare_value_db, double different_value_db);

    /* Check the current value against a condition, and store the value */
    bool check_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db, double different_value_db);

    /* Check the current value against a condition without storing it */
    bool check_no_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db, double different_value_db);

    static int type_to_size();

private:
    static const char* type_to_fmt(bool hex);

};

#endif
