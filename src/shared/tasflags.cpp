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

#include "tasflags.h"

struct TasFlags tasflags = {
    running        : 0,
    speed_divisor  : 1,
    recording      : -1,
    fastforward    : 0,
    includeFlags   : LCF_WINDOW,
    //includeFlags   : LCF_ALL,
    excludeFlags   : LCF_NONE,
    av_dumping     : 0,
    framerate      : 30,
    numControllers : 1
}; 

