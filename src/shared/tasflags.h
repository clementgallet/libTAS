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

#ifndef LIBTAS_TASFLAGS_H_INCLUDED
#define LIBTAS_TASFLAGS_H_INCLUDED

#include "lcf.h"

struct TasFlags {
    /* Is the game running or on pause */
    int running;

    /* By how much is the speed reduced */
    int speed_divisor;

    /* Recording status */
    enum RecStatus {
        NO_RECORDING,
        RECORDING_WRITE,
        RECORDING_READ_WRITE, // Read until end of movie or toggle
        RECORDING_READ_ONLY // Always read, toggle deactivated
    };
    RecStatus recording;

    /* Is fastforward enabled */
    int fastforward;

    /* Which flags trigger a debug message */
    LogCategoryFlag includeFlags;

    /* Which flags prevent triggering a debug message */
    LogCategoryFlag excludeFlags;

    /* Are we dumping audio and video? */
    int av_dumping;

    /* Framerate at which the game is running.
     * Set to 0 to use the nondeterministic timer
     * In that case, AV dumping is disabled.
     */
    unsigned int framerate;

    /* Number of SDL controllers to (virtually) plug in */
    int numControllers;
};

extern struct TasFlags tasflags;

#endif
