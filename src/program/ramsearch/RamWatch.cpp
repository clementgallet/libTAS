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

#include "RamWatch.h"
#include "CompareEnums.h"
#include <cstdint>
#include <sys/types.h>
#include <sys/uio.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <inttypes.h>

bool RamWatch::isValid;
pid_t RamWatch::game_pid;
int RamWatch::type;
int RamWatch::type_size;

const char* RamWatch::type_to_fmt(bool hex)
{
    switch (type) {
        case RamChar:
            return hex?"%hhx":"%hhd";
        case RamShort:
            return hex?"%hx":"%hd";
        case RamInt:
            return hex?"%x":"%d";
        case RamUnsignedChar:
            return hex?"%hhx":"%hhu";
        case RamUnsignedShort:
            return hex?"%hx":"%hu";
        case RamUnsignedInt:
            return hex?"%x":"%u";
        case RamUnsignedLong:
            return hex?"%" PRIx64:"%" PRIu64;
        case RamLong:
            return hex?"%" PRIx64:"%" PRId64;
        case RamFloat:
            return hex?"%a":"%g";
        case RamDouble:
            return hex?"%la":"%lg";
    }
    return "";
}

int RamWatch::type_to_size()
{
    switch (type) {
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

const char* RamWatch::tostring(bool hex, uint64_t value) const
{
    static char str[30];

    switch (type) {
        case RamChar:
        {
            int8_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%d", typed_value);
            return str;
        }
        case RamUnsignedChar:
        {
            uint8_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%u", typed_value);
            return str;
        }
        case RamShort:
        {
            int16_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%d", typed_value);
            return str;
        }
        case RamUnsignedShort:
        {
            uint16_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%u", typed_value);
            return str;
        }
        case RamInt:
        {
            int32_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%d", typed_value);
            return str;
        }
        case RamUnsignedInt:
        {
            uint32_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%x":"%d", typed_value);
            return str;
        }
        case RamLong:
        {
            int64_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%" PRIx64:"%" PRId64, typed_value);
            return str;
        }
        case RamUnsignedLong:
        {
            uint64_t typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%" PRIx64:"%" PRIu64, typed_value);
            return str;
        }
        case RamFloat:
        {
            float typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%a":"%g", typed_value);
            return str;
        }
        case RamDouble:
        {
            double typed_value;
            memcpy(&typed_value, &value, type_size);
            snprintf(str, 30, hex?"%la":"%lg", typed_value);
            return str;
        }
    }
    return str;
}

uint64_t RamWatch::get_value() const
{
    struct iovec local, remote;
    uint64_t value = 0;
    local.iov_base = static_cast<void*>(&value);
    local.iov_len = type_size;
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = type_size;

    isValid = (process_vm_readv(game_pid, &local, 1, &remote, 1, 0) == type_size);

    return value;
}

#define CHECK_NAN(T) \
{\
T typed_value;\
memcpy(&typed_value, &value, type_size);\
if (typed_value != typed_value)\
    return true;\
}

bool RamWatch::query()
{
    uint64_t value = get_value();
    if (!isValid)
        return true;

    previous_value = value;

    /* Check NaN for float/double */
    switch(type) {
        case RamFloat:
            CHECK_NAN(float)
            break;
        case RamDouble:
            CHECK_NAN(double)
            break;
    }

    return false;
}

#define CHECK_TYPED(T) \
{\
T typed_value;\
memcpy(&typed_value, &value, type_size);\
T compare_value;\
if (compare_type == CompareType::Previous)\
    memcpy(&compare_value, &previous_value, type_size);\
else\
    compare_value = static_cast<T>(compare_value_db);\
switch(compare_operator) {\
    case CompareOperator::Equal:\
        return typed_value != compare_value;\
    case CompareOperator::NotEqual:\
        return typed_value == compare_value;\
    case CompareOperator::Less:\
        return typed_value >= compare_value;\
    case CompareOperator::Greater:\
        return typed_value <= compare_value;\
    case CompareOperator::LessEqual:\
        return typed_value > compare_value;\
    case CompareOperator::GreaterEqual:\
        return typed_value < compare_value;\
}\
}

bool RamWatch::check(uint64_t value, CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
{
    switch(type) {
        case RamChar:
            CHECK_TYPED(int8_t)
        case RamUnsignedChar:
            CHECK_TYPED(uint8_t)
        case RamShort:
            CHECK_TYPED(int16_t)
        case RamUnsignedShort:
            CHECK_TYPED(uint16_t)
        case RamInt:
            CHECK_TYPED(int32_t)
        case RamUnsignedInt:
            CHECK_TYPED(uint32_t)
        case RamLong:
            CHECK_TYPED(int64_t)
        case RamUnsignedLong:
            CHECK_TYPED(uint64_t)
        case RamFloat:
            CHECK_NAN(float)
            CHECK_TYPED(float)
        case RamDouble:
            CHECK_NAN(double)
            CHECK_TYPED(double)
    }

    return false;
}

bool RamWatch::check_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
{
    uint64_t value = get_value();
    if (!isValid)
        return true;

    bool res = check(value, compare_type, compare_operator, compare_value_db);
    previous_value = value;
    return res;
}

bool RamWatch::check_no_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
{
    uint64_t value = get_value();
    if (!isValid)
        return true;

    return check(value, compare_type, compare_operator, compare_value_db);
}
