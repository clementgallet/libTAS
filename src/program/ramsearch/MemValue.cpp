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

#include "MemValue.h"

#include <cstdint>
#include <cstdio>
#include <inttypes.h>
#include <sstream>

int MemValue::type_size(int value_type)
{
    switch (value_type) {
        case RamChar:
        case RamUnsignedChar:
            return 1;
        case RamShort:
        case RamUnsignedShort:
            return 2;
        case RamInt:
        case RamUnsignedInt:
        case RamFloat:
            return 4;
        case RamLong:
        case RamUnsignedLong:
        case RamDouble:
            return 8;
    }
    return 0;
}

const char* MemValue::to_string(const void* value, int value_type, bool hex)
{
    static char str[30];
    const MemValueType* v = static_cast<const MemValueType*>(value);

    switch (value_type) {
        case RamChar:
        {
            snprintf(str, 30, hex?"%x":"%d", v->v_int8_t);
            return str;
        }
        case RamUnsignedChar:
        {
            snprintf(str, 30, hex?"%x":"%u", v->v_uint8_t);
            return str;
        }
        case RamShort:
        {
            snprintf(str, 30, hex?"%x":"%d", v->v_int16_t);
            return str;
        }
        case RamUnsignedShort:
        {
            snprintf(str, 30, hex?"%x":"%u", v->v_uint16_t);
            return str;
        }
        case RamInt:
        {
            snprintf(str, 30, hex?"%x":"%d", v->v_int32_t);
            return str;
        }
        case RamUnsignedInt:
        {
            snprintf(str, 30, hex?"%x":"%u", v->v_uint32_t);
            return str;
        }
        case RamLong:
        {
            snprintf(str, 30, hex?"%" PRIx64:"%" PRId64, v->v_int64_t);
            return str;
        }
        case RamUnsignedLong:
        {
            snprintf(str, 30, hex?"%" PRIx64:"%" PRIu64, v->v_uint64_t);
            return str;
        }
        case RamFloat:
        {
            snprintf(str, 30, hex?"%a":"%g", v->v_float);
            return str;
        }
        case RamDouble:
        {
            snprintf(str, 30, hex?"%la":"%lg", v->v_double);
            return str;
        }
    }
    return str;
}


MemValueType MemValue::from_string(const char* str, int value_type, bool hex)
{
    MemValueType value;
    std::istringstream iss(str);
    if (hex) iss >> std::hex;

    switch (value_type) {
        case RamChar:
        {
            /* ISS will consider char and unsigned char as text element. To get the
             * integer value, I need to cast it to another integer type. */
            int intval;
            iss >> intval;
            value.v_int8_t = static_cast<int8_t>(intval);
            return value;
        }
        case RamUnsignedChar:
        {
            int intval;
            iss >> intval;
            value.v_uint8_t = static_cast<uint8_t>(intval);
            return value;
        }
        case RamShort:
        {
            iss >> value.v_int16_t;
            return value;
        }
        case RamUnsignedShort:
        {
            iss >> value.v_uint16_t;
            return value;
        }
        case RamInt:
        {
            iss >> value.v_int32_t;
            return value;
        }
        case RamUnsignedInt:
        {
            iss >> value.v_uint32_t;
            return value;
        }
        case RamLong:
        {
            iss >> value.v_int64_t;
            return value;
        }
        case RamUnsignedLong:
        {
            iss >> value.v_uint64_t;
            return value;
        }
        case RamFloat:
        {
            iss >> value.v_float;
            return value;
        }
        case RamDouble:
        {
            iss >> value.v_double;
            return value;
        }
    }
    
    return value;
}
