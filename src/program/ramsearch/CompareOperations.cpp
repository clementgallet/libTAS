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
#include <cstdio>
#include <inttypes.h>

typedef union {
    int8_t v_int8_t;
    uint8_t v_uint8_t;
    int16_t v_int16_t;
    uint16_t v_uint16_t;
    int32_t v_int32_t;
    uint32_t v_uint32_t;
    int64_t v_int64_t;
    uint64_t v_uint64_t;
    float v_float;
    double v_double;
} value_t;

/* Cast once the compared values to the appropriate type */
static value_t compare_value;
static value_t different_value;

typedef bool (*compare_t)(const value_t*);
static compare_t compare_method;

static int value_type;

#define DEFINE_CHECK_TYPED(T) \
static bool check_equal_##T(const value_t* value) \
{\
    return value->v_##T == compare_value.v_##T;\
}\
static bool check_notequal_##T(const value_t* value) \
{\
    return value->v_##T != compare_value.v_##T;\
}\
static bool check_less_##T(const value_t* value) \
{\
    return value->v_##T < compare_value.v_##T;\
}\
static bool check_greater_##T(const value_t* value) \
{\
    return value->v_##T > compare_value.v_##T;\
}\
static bool check_lessequal_##T(const value_t* value) \
{\
    return value->v_##T <= compare_value.v_##T;\
}\
static bool check_greaterequal_##T(const value_t* value) \
{\
    return value->v_##T>= compare_value.v_##T;\
}\
static bool check_different_##T(const value_t* value) \
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
compare_value.v_##T = static_cast<T>(compare_value_db);\
different_value.v_##T = static_cast<T>(different_value_db);\
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

bool CompareOperations::check_value(const void* value)
{
    return compare_method(static_cast<const value_t*>(value));
}

bool CompareOperations::check_previous(const void* value, const void* old_value)
{
    compare_value = *static_cast<const value_t*>(old_value);
    return compare_method(static_cast<const value_t*>(value));
}

const char* CompareOperations::tostring(const void* value, bool hex)
{
    static char str[30];
    const value_t* v = static_cast<const value_t*>(value);

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
