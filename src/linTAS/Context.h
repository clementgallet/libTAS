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

#ifndef LINTAS_CONTEXT_H_INCLUDED
#define LINTAS_CONTEXT_H_INCLUDED

#include <string>
#include "Config.h"
#include <X11/Xlib.h>

struct Context {
    /* Execution status */
    enum RunStatus {
        INACTIVE,
        STARTING,
        ACTIVE,
        QUITTING
    };
    RunStatus status = INACTIVE;

    /* Connection to the X server */
    Display *display;

    /* Game window */
    Window game_window = 0;

    /* frame count */
    unsigned long int framecount = 0;

    /* Recording status */
    enum RecStatus {
        NO_RECORDING,
        RECORDING_WRITE,
        RECORDING_READ_WRITE, // Read until end of movie or toggle
        RECORDING_READ_ONLY // Always read, toggle deactivated
    };
    RecStatus recording = NO_RECORDING;

    /* config */
    Config config;

    /* Absolute path of libTAS.so */
    std::string libtaspath;

    /* Absolute path of the game executable */
    std::string gamepath;
};

#endif
