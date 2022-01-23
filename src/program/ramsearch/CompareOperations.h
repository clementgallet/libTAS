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

#ifndef LIBTAS_COMPAREOPERATIONS_H_INCLUDED
#define LIBTAS_COMPAREOPERATIONS_H_INCLUDED

#include <cstdint>
// #include <sys/types.h>

enum class CompareType {
    Previous,
    Value,
};

enum class CompareOperator {
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Different,
};

namespace CompareOperations {

    void init(int value_type, CompareOperator compare_operator, double compare_value_db, double different_value_db);

    /* Compute the comparaison between the content of value and the stored contant value */
    bool check_value(const void* value);

    /* Compute the comparaison between the content of value and the old value */
    bool check_previous(const void* value, const void* old_value);
    
    /* Format a value to be shown */
    const char* tostring(const void* value, bool hex);
}

#endif
