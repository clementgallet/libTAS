/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_RAMWATCH_H_INCLUDED
#define LINTAS_RAMWATCH_H_INCLUDED

#include "CompareEnums.h"
#include "IRamWatch.h"
#include <cstdint>
#include <sys/types.h>
#include <sys/uio.h>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <inttypes.h>
#include <cmath> // std::isfinite

template <typename T> static inline const char* fmt_from_type(bool hex) {return hex?"%x":"%d";}
template <> inline const char* fmt_from_type<float>(bool hex) {return hex?"%a":"%g";}
template <> inline const char* fmt_from_type<double>(bool hex) {return hex?"%la":"%lg";}
template <> inline const char* fmt_from_type<int64_t>(bool hex) {return hex?"%" PRIx64:"%" PRId64;}
template <> inline const char* fmt_from_type<uint64_t>(bool hex) {return hex?"%" PRIx64:"%" PRIu64;}

template <class T>
class RamWatch : public virtual IRamWatch {
public:
    T previous_value;

    RamWatch(uintptr_t addr) : IRamWatch(addr) {};

    const char* tostring(bool hex)
    {
        static char str[30];
        /* Use snprintf instead of ostringstream for a good speedup */
        snprintf(str, 30, fmt_from_type<T>(hex), previous_value);
        return str;
    }

    const char* tostring_current(bool hex)
    {
        static char str[30];
        /* Use snprintf instead of ostringstream for a good speedup */
        snprintf(str, 30, fmt_from_type<T>(hex), get_value());
        return str;
    }

    T get_value()
    {
        struct iovec local, remote;
        T value = 0;
        local.iov_base = static_cast<void*>(&value);
        local.iov_len = sizeof(T);
        remote.iov_base = reinterpret_cast<void*>(address);
        remote.iov_len = sizeof(T);

        last_read = process_vm_readv(game_pid, &local, 1, &remote, 1, 0);

        return value;
    }

    bool query()
    {
        T value = get_value(); // sets last_read == -1 if error
        if (last_read == -1)
            return true;

        previous_value = value;

        /* Check NaN/Inf for float/double */
        if (!std::isfinite(value))
            return true;

        return false;
    }

    bool check(T value, CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
    {
        /* Check NaN/Inf for float/double */
        if (!std::isfinite(value))
            return true;

        T compare_value;

        if (compare_type == CompareType::Previous) {
            compare_value = previous_value;
        }
        if (compare_type == CompareType::Value) {
            compare_value = static_cast<T>(compare_value_db);
        }

        switch(compare_operator) {
            case CompareOperator::Equal:
                if (value != compare_value)
                    return true;
                break;
            case CompareOperator::NotEqual:
                if (value == compare_value)
                    return true;
                break;
            case CompareOperator::Less:
                if (value >= compare_value)
                    return true;
                break;
            case CompareOperator::Greater:
                if (value <= compare_value)
                    return true;
                break;
            case CompareOperator::LessEqual:
                if (value > compare_value)
                    return true;
                break;
            case CompareOperator::GreaterEqual:
                if (value < compare_value)
                    return true;
                break;
        }

        return false;
    }

    bool check_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
    {
        T value = get_value(); // sets last_read == -1 if error
        if (last_read == -1)
            return true;

        bool res = check(value, compare_type, compare_operator, compare_value_db);
        previous_value = value;
        return res;
    }

    bool check_no_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
    {
        T value = get_value(); // sets last_read == -1 if error
        if (last_read == -1)
            return true;

        return check(value, compare_type, compare_operator, compare_value_db);
    }
};

#endif
