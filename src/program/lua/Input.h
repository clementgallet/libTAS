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

#ifndef LIBTAS_LUAINPUT_H_INCLUDED
#define LIBTAS_LUAINPUT_H_INCLUDED

extern "C" {
#include <lua.h>
}

class AllInputs;

namespace Lua {

namespace Input {

    /* Register all functions */
    void registerFunctions(lua_State *L);

    /* Pass the current AllInputs object to be used by lua functions */
    void registerInputs(AllInputs* ai);

    /* Clear the input state */
    int clear(lua_State *L);

    /* Set key (number keysym, number state) */
    int setKey(lua_State *L);

    /* Get key (number keysym) -> number state */
    int getKey(lua_State *L);

    /* Set mouse coords (number x, number y, number mode) */
    int setMouseCoords(lua_State *L);

    /* Get mouse coords () -> number x, number y, number mode */
    int getMouseCoords(lua_State *L);

    /* Set mouse button (number button, number state) */
    int setMouseButtons(lua_State *L);

    /* Get mouse coords (number button) -> number state */
    int getMouseButtons(lua_State *L);

    /* Set controller button (number controller, number button, number state) */
    int setControllerButton(lua_State *L);
    
    /* Get controller button (number controller, number button) -> number */
    int getControllerButton(lua_State *L);

    /* Set controller axis (number controller, number axis, number value) */
    int setControllerAxis(lua_State *L);
    
    /* Get controller axis (number controller, number axis) -> number */
    int getControllerAxis(lua_State *L);

    /* Set flag (number flag, number state) */
    int setFlag(lua_State *L);
    
    /* Get flag (number flag) -> number state */
    int getFlag(lua_State *L);

    /* Set framerate (number num, number den) */
    int setFramerate(lua_State *L);

    /* Get framerate () -> number num, number den */
    int getFramerate(lua_State *L);

    /* Set realtime (number sec, number nsec) */
    int setRealtime(lua_State *L);

    /* Get realtime () -> number sec, number nsec */
    int getRealtime(lua_State *L);

}
}

#endif
