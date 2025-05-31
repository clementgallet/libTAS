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

#include "CompareOperations.h"
#include "MemValue.h"

#include <cstdint>
#include <cstdio>
#include <inttypes.h>
#include <cstring>

/* Cast once the compared values to the appropriate type */
static MemValueType compare_value;
static MemValueType different_value;

typedef bool (*compare_t)(const MemValueType*);
static compare_t compare_method;

static int value_type;

#define DEFINE_CHECK_TYPED(T) \
static bool check_equal_##T(const MemValueType* value) \
{\
    return value->v_##T == compare_value.v_##T;\
}\
static bool check_notequal_##T(const MemValueType* value) \
{\
    return value->v_##T != compare_value.v_##T;\
}\
static bool check_less_##T(const MemValueType* value) \
{\
    return value->v_##T < compare_value.v_##T;\
}\
static bool check_greater_##T(const MemValueType* value) \
{\
    return value->v_##T > compare_value.v_##T;\
}\
static bool check_lessequal_##T(const MemValueType* value) \
{\
    return value->v_##T <= compare_value.v_##T;\
}\
static bool check_greaterequal_##T(const MemValueType* value) \
{\
    return value->v_##T>= compare_value.v_##T;\
}\
static bool check_different_##T(const MemValueType* value) \
{\
    return (value->v_##T - compare_value.v_##T) == different_value.v_##T;\
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

static bool check_equal_array(const MemValueType* value)
{
    return 0 == memcmp(value->v_array, compare_value.v_array, compare_value.v_array[RAM_ARRAY_MAX_SIZE]);
}

static bool check_equal_string(const MemValueType* value)
{
    return 0 == strncmp(value->v_cstr, compare_value.v_cstr, RAM_ARRAY_MAX_SIZE);
}

void CompareOperations::init(int vt, CompareOperator compare_operator, MemValueType compare_v, MemValueType different_v)
{
    value_type = vt;
    compare_value = compare_v;
    different_value = different_v;
    
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
        case RamArray:
            compare_method = check_equal_array;
            break;
        case RamCString:
            compare_method = check_equal_string;
            break;
    }
}

bool CompareOperations::check_value(const void* value)
{
    return compare_method(static_cast<const MemValueType*>(value));
}

bool CompareOperations::check_previous(const void* value, const void* old_value)
{
    compare_value = *static_cast<const MemValueType*>(old_value);
    return compare_method(static_cast<const MemValueType*>(value));
}
