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

#include "CompareOperations.h"
#include "TypeIndex.h"
#include <cstdint>
// #include <sys/types.h>
// #include <cerrno>
// #include <cstring>
// #include <cstdio>
// #include <inttypes.h>


/* Cast once the compared values to the appropriate type */
static int8_t compare_value_int8_t;
static uint8_t compare_value_uint8_t;
static int16_t compare_value_int16_t;
static uint16_t compare_value_uint16_t;
static int32_t compare_value_int32_t;
static uint32_t compare_value_uint32_t;
static int64_t compare_value_int64_t;
static uint64_t compare_value_uint64_t;
static float compare_value_float;
static double compare_value_double;

static int8_t different_value_int8_t;
static uint8_t different_value_uint8_t;
static int16_t different_value_int16_t;
static uint16_t different_value_uint16_t;
static int32_t different_value_int32_t;
static uint32_t different_value_uint32_t;
static int64_t different_value_int64_t;
static uint64_t different_value_uint64_t;
static float different_value_float;
static double different_value_double;

typedef bool (*compare_t)(void*);
static compare_t compare_method;

static int value_type;

#define DEFINE_CHECK_TYPED(T) \
static bool check_equal_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value == compare_value_##T;\
}\
static bool check_notequal_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value != compare_value_##T;\
}\
static bool check_less_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value < compare_value_##T;\
}\
static bool check_greater_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value > compare_value_##T;\
}\
static bool check_lessequal_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value <= compare_value_##T;\
}\
static bool check_greaterequal_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return typed_value >= compare_value_##T;\
}\
static bool check_different_##T(void* value) \
{\
    T typed_value = *reinterpret_cast<T*>(value);\
    return (typed_value-compare_value_##T) == different_value_##T;\
}\

DEFINE_CHECK_TYPED(int8_t)
DEFINE_CHECK_TYPED(uint8_t)
DEFINE_CHECK_TYPED(int16_t)
DEFINE_CHECK_TYPED(uint16_t)
DEFINE_CHECK_TYPED(int32_t)
DEFINE_CHECK_TYPED(uint32_t)
DEFINE_CHECK_TYPED(int64_t)
DEFINE_CHECK_TYPED(uint64_t)
DEFINE_CHECK_TYPED(float)
DEFINE_CHECK_TYPED(double)

#define DEFINE_COMPARE_METHOD_TYPED(T) \
compare_value_##T = static_cast<T>(compare_value_db);\
different_value_##T = static_cast<T>(different_value_db);\
switch(compare_operator) {\
    case CompareOperator::Equal:\
        compare_method = &check_equal_##T;\
        break;\
    case CompareOperator::NotEqual:\
        compare_method = &check_notequal_##T;\
        break;\
    case CompareOperator::Less:\
        compare_method = &check_less_##T;\
        break;\
    case CompareOperator::Greater:\
        compare_method = &check_greater_##T;\
        break;\
    case CompareOperator::LessEqual:\
        compare_method = &check_lessequal_##T;\
        break;\
    case CompareOperator::GreaterEqual:\
        compare_method = &check_greaterequal_##T;\
        break;\
    case CompareOperator::Different:\
        compare_method = &check_different_##T;\
        break;\
}\

void CompareOperations::init(int vt, CompareOperator compare_operator, double compare_value_db, double different_value_db)
{
    value_type = vt;
    
    /* Initialize the comparaison method and values */
    switch(value_type) {
        case RamChar:
            DEFINE_COMPARE_METHOD_TYPED(int8_t)
            break;
        case RamUnsignedChar:
            DEFINE_COMPARE_METHOD_TYPED(uint8_t)
            break;
        case RamShort:
            DEFINE_COMPARE_METHOD_TYPED(int16_t)
            break;
        case RamUnsignedShort:
            DEFINE_COMPARE_METHOD_TYPED(uint16_t)
            break;
        case RamInt:
            DEFINE_COMPARE_METHOD_TYPED(int32_t)
            break;
        case RamUnsignedInt:
            DEFINE_COMPARE_METHOD_TYPED(uint32_t)
            break;
        case RamLong:
            DEFINE_COMPARE_METHOD_TYPED(int64_t)
            break;
        case RamUnsignedLong:
            DEFINE_COMPARE_METHOD_TYPED(uint64_t)
            break;
        case RamFloat:
            DEFINE_COMPARE_METHOD_TYPED(float)
            break;
        case RamDouble:
            DEFINE_COMPARE_METHOD_TYPED(double)
            break;
    }
}

bool CompareOperations::check_value(void* value)
{
    return compare_method(value);
}

#define STORE_OLD_VALUE_TYPED(T)\
compare_value_##T = *reinterpret_cast<T*>(old_value);\

bool CompareOperations::check_previous(void* value, void* old_value)
{
    switch(value_type) {
        case RamChar:
            STORE_OLD_VALUE_TYPED(int8_t)
            break;
        case RamUnsignedChar:
            STORE_OLD_VALUE_TYPED(uint8_t)
            break;
        case RamShort:
            STORE_OLD_VALUE_TYPED(int16_t)
            break;
        case RamUnsignedShort:
            STORE_OLD_VALUE_TYPED(uint16_t)
            break;
        case RamInt:
            STORE_OLD_VALUE_TYPED(int32_t)
            break;
        case RamUnsignedInt:
            STORE_OLD_VALUE_TYPED(uint32_t)
            break;
        case RamLong:
            STORE_OLD_VALUE_TYPED(int64_t)
            break;
        case RamUnsignedLong:
            STORE_OLD_VALUE_TYPED(uint64_t)
            break;
        case RamFloat:
            STORE_OLD_VALUE_TYPED(float)
            break;
        case RamDouble:
            STORE_OLD_VALUE_TYPED(double)
            break;
    }
    return compare_method(value);
}
