/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_LUARUNTIME_H_INCLUDED
#define LIBTAS_LUARUNTIME_H_INCLUDED

extern "C" {
#include <lua.h>
}

struct Context;

namespace Lua {

namespace Runtime {

    /* Register all functions */
    void registerFunctions(lua_State *L, Context* c);

    /* Perform a savestate */
    int saveState(lua_State *L);

    /* Load a savestate */
    int loadState(lua_State *L);

    /* Is fast-forward set */
    int isFastForward(lua_State *L);

    /* Get the elapsed time since the game startup */
    int setFastForward(lua_State *L);

    /* Sleep the given number of milliseconds */
    int sleepMS(lua_State *L);

}
}

#endif
