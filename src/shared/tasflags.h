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

#ifndef TASFLAGS_H_INCLUDED
#define TASFLAGS_H_INCLUDED

#include "lcf.h"

struct TasFlags {
    int running; // is the game running or on pause
    int speed_divisor; // by how much is the speed reduced
    int recording; // are the input recorded or read
    int fastforward; // is fastforward enabled
    LogCategoryFlag includeFlags; // which flags trigger a debug message
    LogCategoryFlag excludeFlags; // which flags prevent triggering a debug message
    int av_dumping; // Are we dumping audio and video?
    unsigned int framerate; // Framerate at which the game is running. Set to 0 to use the nondeterministic timer
    int numControllers; // Number of SDL controllers to (virtually) plug in
};

extern struct TasFlags tasflags;

#endif // TAS_FLAGS_H_INCLUDED
