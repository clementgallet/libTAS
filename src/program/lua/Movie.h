/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_LUAMOVIE_H_INCLUDED
#define LIBTAS_LUAMOVIE_H_INCLUDED

extern "C" {
#include <lua.h>
}

#include <string>

struct Context;

namespace Lua {

namespace Movie {

    /* Register all functions */
    void registerFunctions(lua_State *L, Context* c);

    /* Get the current framecount */
    int currentFrame(lua_State *L);

    /* Get the movie framecount */
    int frameCount(lua_State *L);

    /* Get the movie status (nomovie, playback, recording) */
    int status(lua_State *L);

    /* Get the elapsed time since the game startup */
    int time(lua_State *L);

    /* Get the movie rerecord count */
    int rerecords(lua_State *L);

    /* Returns if the current frame is a draw frame */
    int isDraw(lua_State *L);

    /* Get marker at current frame if there is one */
    int getMarker(lua_State *L);
}
}

#endif
