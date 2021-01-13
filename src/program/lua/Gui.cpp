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

#include "Gui.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <iostream>
#include <string>
#include <lua.h>
#include <lauxlib.h>

static Context* context;

/* List of functions to register */
static const luaL_Reg gui_functions[] =
{
    { "text", Lua::Gui::text},
    { NULL, NULL }
};

void Lua::Gui::registerFunctions(Context* c)
{
    context = c;
    luaL_newlib(context->lua_state, gui_functions);
    lua_setglobal(context->lua_state, "gui");
}

int Lua::Gui::text(lua_State *L)
{
    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    std::string text = luaL_checklstring(L, 3, nullptr);
    uint32_t fg_color = luaL_optnumber (L, 4, 0x00ffffff);
    uint32_t bg_color = luaL_optnumber (L, 5, 0x00000000);
    
    sendMessage(MSGN_LUA_TEXT);
    sendData(&x, sizeof(int));
    sendData(&y, sizeof(int));
    sendString(text);
    sendData(&fg_color, sizeof(uint32_t));
    sendData(&bg_color, sizeof(uint32_t));
    
    return 0;
}
