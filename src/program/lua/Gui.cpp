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
#include "../../shared/sockethelpers.h"
#include "../../shared/messages.h"

#include <iostream>
#include <string>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static Context* context;

/* List of functions to register */
static const luaL_Reg gui_functions[] =
{
    { "resolution", Lua::Gui::resolution},
    { "text", Lua::Gui::text},
    { "pixel", Lua::Gui::pixel},
    { "rectangle", Lua::Gui::rectangle},
    { NULL, NULL }
};

void Lua::Gui::registerFunctions(Context* c)
{
    context = c;
    luaL_newlib(context->lua_state, gui_functions);
    lua_setglobal(context->lua_state, "gui");
}

/* Get the window resolution */
int Lua::Gui::resolution(lua_State *L)
{
    sendMessage(MSGN_LUA_RESOLUTION);
    
    /* Get the resolution */
    int message = receiveMessage();
    if (message != MSGB_LUA_RESOLUTION) {
        std::cerr << "Wrong result after asking for lua game resolution" << std::endl;
        exit(1);
    }
    int w, h;
    receiveData(&w, sizeof(int));
    receiveData(&h, sizeof(int));    
    lua_pushinteger(L, static_cast<lua_Integer>(w));
    lua_pushinteger(L, static_cast<lua_Integer>(h));
    return 2;
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

int Lua::Gui::pixel(lua_State *L)
{
    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    uint32_t color = luaL_optnumber (L, 3, 0x00ffffff);
    
    sendMessage(MSGN_LUA_PIXEL);
    sendData(&x, sizeof(int));
    sendData(&y, sizeof(int));
    sendData(&color, sizeof(uint32_t));
    
    return 0;
}

int Lua::Gui::rectangle(lua_State *L)
{
    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    int w = static_cast<int>(lua_tointeger(L, 3));
    int h = static_cast<int>(lua_tointeger(L, 4));
    int thickness = luaL_optnumber (L, 5, 1);
    uint32_t outline_color = luaL_optnumber (L, 6, 0x00ffffff);
    uint32_t fill_color = luaL_optnumber (L, 7, 0xffffffff);
    
    sendMessage(MSGN_LUA_RECT);
    sendData(&x, sizeof(int));
    sendData(&y, sizeof(int));
    sendData(&w, sizeof(int));
    sendData(&h, sizeof(int));
    sendData(&thickness, sizeof(int));
    sendData(&outline_color, sizeof(uint32_t));
    sendData(&fill_color, sizeof(uint32_t));
    
    return 0;
}
