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
        QUITTING,
        RESTARTING
    };
    volatile RunStatus status = INACTIVE;

    /* Connection to the X server */
    Display *display;

    /* Game window */
    Window game_window = 0;

    /* Main UI window */
    Window ui_window = 0;

    /* Recording status */
    enum FocusState {
        FOCUS_GAME = 0x01,
        FOCUS_UI = 0x02,
        FOCUS_ALL = 0x04
    };

    /* When are hotkeys/inputs accepted */
    int hotkeys_focus = FOCUS_GAME;
    int inputs_focus = FOCUS_GAME;

    /* frame count */
    unsigned long int framecount = 0;

    /* config */
    Config config;

    /* Absolute path of libTAS.so */
    std::string libtaspath;

    /* Absolute path of the game executable */
    std::string gamepath;

    /* Name of the game executable */
    std::string gamename;

    /* PID of the game */
    pid_t game_pid;

    /* Attaching gdb? */
    bool attach_gdb = false;
};

#endif
