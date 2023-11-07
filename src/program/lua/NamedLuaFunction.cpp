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

#include "NamedLuaFunction.h"
#include "Main.h"

extern "C" {
#include <lauxlib.h>
}
#include <iostream>

namespace Lua {

NamedLuaFunction::NamedLuaFunction(lua_State *L, CallbackType t) : type(t), file(Main::currentFile()), active(true), lua_state(L)
{
    function_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
}

NamedLuaFunction::~NamedLuaFunction()
{
    if (function_ref)
        luaL_unref(lua_state, LUA_REGISTRYINDEX, function_ref);
    function_ref = 0;
}

/* Taken from https://stackoverflow.com/a/21947358 */
void NamedLuaFunction::call()
{
    /* push the callback onto the stack using the Lua reference we */
    /* stored in the registry */
    lua_rawgeti( lua_state, LUA_REGISTRYINDEX, function_ref );

    /* call the callback */
    if ( 0 != lua_pcall( lua_state, 0, 0, 0 ) ) {
        std::cerr << "Failed to call the callback: " << lua_tostring( lua_state, -1 ) << std::endl;
        lua_pop(lua_state, 1);
        return;
    }
}

}
