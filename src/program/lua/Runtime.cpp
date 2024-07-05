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

#include "Runtime.h"

#include "Context.h"

#include <unistd.h>
#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static Context* context;

/* List of functions to register */
static const luaL_Reg runtime_functions[] =
{
    { "saveState", Lua::Runtime::saveState},
    { "loadState", Lua::Runtime::loadState},
    { "isFastForward", Lua::Runtime::isFastForward},
    { "setFastForward", Lua::Runtime::setFastForward},
    { "sleepMS", Lua::Runtime::sleepMS},
    { NULL, NULL }
};

void Lua::Runtime::registerFunctions(lua_State *L, Context* c)
{
    context = c;
    luaL_newlib(L, runtime_functions);
    lua_setglobal(L, "runtime");
}

int Lua::Runtime::saveState(lua_State *L)
{
    int slot = static_cast<int>(lua_tointeger(L, 1));
    if (slot >= 1 && slot <= 10)
        context->hotkey_pressed_queue.push(HOTKEY_SAVESTATE1 + (slot-1));
    return 0;
}

int Lua::Runtime::loadState(lua_State *L)
{
    int slot = static_cast<int>(lua_tointeger(L, 1));
    if (slot >= 1 && slot <= 10)
        context->hotkey_pressed_queue.push(HOTKEY_LOADSTATE1 + (slot-1));
    return 0;
}

int Lua::Runtime::isFastForward(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(context->config.sc.fastforward));
    return 1;
}

int Lua::Runtime::setFastForward(lua_State *L)
{
    context->config.sc.fastforward = static_cast<int>(lua_tointeger(L, 1));
    context->config.sc_modified = true;
    return 0;
}

int Lua::Runtime::sleepMS(lua_State *L)
{
    int length = static_cast<int>(lua_tointeger(L, 1));
    if (length >= 0)
        usleep(length);
    return 0;
}
