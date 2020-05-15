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

#ifndef LIBTAS_CONTEXT_H_INCLUDED
#define LIBTAS_CONTEXT_H_INCLUDED

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
        RESTARTING,
    };
    volatile RunStatus status = INACTIVE;

    /* Connection to the X server */
    xcb_connection_t *conn;

    /* Game window */
    xcb_window_t game_window = 0;

    /* Main UI window */
    xcb_window_t ui_window = 0;

    /* Crosshair cursor */
    xcb_cursor_t crosshair_cursor;

    /* Recording status */
    enum FocusState {
        FOCUS_GAME = 0x01,
        FOCUS_UI = 0x02,
        FOCUS_ALL = 0x04
    };

    /* frame count */
    uint64_t framecount = 0;

    /* current time */
    int64_t current_time_sec;
    int64_t current_time_nsec;

    /* movie time */
    uint64_t movie_time_sec;
    uint64_t movie_time_nsec;

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
    uint64_t pause_frame = 0;

    /* Can we use incremental savestates? */
    bool is_soft_dirty = false;

    /* MD5 hash of the game executable */
    std::string md5_game;

    /* MD5 hash of the game executable that is stored in the movie */
    std::string md5_movie;

    /* Current encoding segment. Sent when game is restarted */
    int encoding_segment = 0;

    /* Content of LD_PRELOAD when libTAS is executed */
    std::string old_ld_preload;

    /* fps values */
    float fps, lfps = -1;
};

#endif
