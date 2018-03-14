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

#ifndef LINTAS_CONTEXT_H_INCLUDED
#define LINTAS_CONTEXT_H_INCLUDED

#include <string>
#include "Config.h"
// #include <X11/Xlib.h>
#include <xcb/xcb.h>
#include "ConcurrentQueue.h"
#include "../shared/GameInfo.h"

struct Context {
    /* Execution status */
    enum RunStatus {
        INACTIVE,
        STARTING,
        ACTIVE,
        QUITTING,
    };
    volatile RunStatus status = INACTIVE;

    /* Connection to the X server */
    xcb_connection_t *conn;

    /* Game window */
    xcb_window_t game_window = 0;

    /* Main UI window */
    xcb_window_t ui_window = 0;

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
    unsigned int framecount = 0;

    /* current time */
    struct timespec current_time;

    /* time at the end of the loaded movie */
    struct timespec movie_end_time;

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

    /* Rerecord count */
    unsigned int rerecord_count = 0;

    /* Queue of hotkeys that where pushed by the UI, to process by the main thread */
    ConcurrentQueue<HotKeyType> hotkey_queue;

    /* Store some game information sent by the game, that is shown in the UI */
    GameInfo game_info;

    /* Authors of the movie */
    std::string authors;

    /* A frame number when the game pauses */
    int pause_frame = 0;

};

#endif
