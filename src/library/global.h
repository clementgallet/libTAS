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

#ifndef LIBTAS_GLOBAL_H_INCL
#define LIBTAS_GLOBAL_H_INCL

#include "../shared/SharedConfig.h"
#include "../shared/GameInfo.h"
#include <X11/Xlib.h>
#include <SDL2/SDL.h>
#include <string>
#include <list>

/* Include this file in every source code that override functions of the game */

#if __GNUC__ >= 4
    #define OVERRIDE extern "C" __attribute__ ((visibility ("default")))
#else
    #define OVERRIDE extern "C"
#endif

#define GAMEDISPLAYNUM 10

namespace libtas {

    /* Configuration parameters that are shared between the program and the game */
    extern SharedConfig shared_config;

    /* Some informations about the game that are send to the program */
    extern GameInfo game_info;

    /* Indicate if the game is exiting. It helps avoiding some invalid or blocking calls */
    extern volatile bool is_exiting;

    /* Do we skip all rendering functions for the current frame */
    extern bool skipping_draw;

    /* Connections to the Xlib server */
    extern Display* gameDisplays[GAMEDISPLAYNUM];

    /* Game window (we suppose there is only one) */
    extern std::list<Window> gameXWindows;

    /* SDL2 game window (we suppose there is only one) */
    extern SDL_Window* gameSDLWindow;

    /* Should we perform a backtrack savestate? */
    extern bool saveBacktrack;
}

#endif
