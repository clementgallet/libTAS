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
    virtual ~IRamWatch() = default;
    virtual std::string get_line() = 0;
    virtual std::string get_line_update() = 0;
    virtual bool search(CompareType compare_type, CompareOperator compare_operator, double compare_value_db) = 0;

    uintptr_t address;
    static ssize_t last_read;
    static pid_t game_pid;
};

#endif
