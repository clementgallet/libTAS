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

#ifndef LIBTAS_IRAMWATCH_H_INCLUDED
#define LIBTAS_IRAMWATCH_H_INCLUDED

#include "CompareEnums.h"
#include <cstdint>
#include <sys/types.h>
#include <string>

class IRamWatch {
public:
    uintptr_t address;
    static bool isValid;
    static pid_t game_pid;

    IRamWatch(uintptr_t addr) : address(addr) {};
    virtual ~IRamWatch() = default;

    /* Format the stored number into a string */
    virtual const char* tostring(bool hex) = 0;

    /* Fetch the current value and format into a string */
    virtual const char* tostring_current(bool hex) = 0;

    /* Check the current value against a condition, and store the value */
    virtual bool check_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db) = 0;

    /* Check the current value against a condition without storing it */
    virtual bool check_no_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db) = 0;

    /* Fetch and store the current value */
    virtual bool query() = 0;

    /* Returns the index of the stored type */
    virtual int type() = 0;
};

#endif
