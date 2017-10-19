/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_IRAMWATCH_H_INCLUDED
#define LINTAS_IRAMWATCH_H_INCLUDED

#include "CompareEnums.h"
#include <cstdint>
#include <sys/types.h>
#include <string>

class IRamWatch {
public:
    uintptr_t address;
    static ssize_t last_read;
    static pid_t game_pid;

    IRamWatch(uintptr_t addr) : address(addr) {};
    virtual ~IRamWatch() = default;
    virtual const char* tostring(bool hex) = 0;
    virtual const char* tostring_current(bool hex) = 0;
    virtual bool check_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db) = 0;
    virtual bool check_no_update(CompareType compare_type, CompareOperator compare_operator, double compare_value_db) = 0;
    virtual bool query() = 0;
};

#endif
