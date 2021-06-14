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

#include "Config.h"

#ifdef __unix__
#include <xcb/xcb.h>
#elif defined(__APPLE__) && defined(__MACH__)
#endif

#include <string>
#include <stdint.h>
#include "ConcurrentQueue.h"
#include "KeyMapping.h"
#include "../shared/GameInfo.h"
extern "C" {
#include <lua.h>
}

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

#ifdef __unix__
    /* Connection to the X server */
    xcb_connection_t *conn;

    /* Game window */
    xcb_window_t game_window = 0;

    /* Main UI window */
    xcb_window_t ui_window = 0;
#elif defined(__APPLE__) && defined(__MACH__)
    /* Dummy */
    unsigned int game_window = 0;
#endif

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
    int64_t movie_time_sec;
    int64_t movie_time_nsec;

    /* config */
    Config config;

    /* Absolute path of libtas.so */
    std::string libtaspath;

    /* Absolute path of libtas32.so */
    std::string libtas32path;

    /* Absolute path of the game executable */
    std::string gamepath;

    /* Name of the game executable */
    std::string gamename;

    /* PID of the forked `sh` process which executes the game */
    pid_t fork_pid;

    /* PID of the game */
    pid_t game_pid;

    /* Attaching gdb? */
    bool attach_gdb = false;

    /* Rerecord count */
    unsigned int rerecord_count = 0;

    /* Queue of pressed hotkeys that where pushed by the UI, to process by the main thread */
    ConcurrentQueue<HotKeyType> hotkey_pressed_queue;

    /* Queue of released hotkeys that where pushed by the UI, to process by the main thread */
    ConcurrentQueue<HotKeyType> hotkey_released_queue;

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

    /* Interactive mode */
    bool interactive = true;
    
    /* Lua state */
    lua_State *lua_state = nullptr;
    
    /* Indicate if at least one savestate was performed, for backtrack savestate */
    bool didASavestate = false;

    /* Socket filename */
    std::string socket_filename;
};

#endif
