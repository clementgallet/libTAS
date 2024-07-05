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

#include "Gui.h"

#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <iostream>
#include <string>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

/* List of functions to register */
static const luaL_Reg gui_functions[] =
{
    { "resolution", Lua::Gui::resolution},
    { "text", Lua::Gui::text},
    { "window", Lua::Gui::window},
    { "pixel", Lua::Gui::pixel},
    { "rectangle", Lua::Gui::rectangle},
    { "line", Lua::Gui::line},
    { "quad", Lua::Gui::quad},
    { "ellipse", Lua::Gui::ellipse},
    { NULL, NULL }
};

void Lua::Gui::registerFunctions(lua_State *L)
{
    luaL_newlib(L, gui_functions);
    lua_setglobal(L, "gui");
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
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    std::string text = luaL_checklstring(L, 3, nullptr);
    uint32_t color = luaL_optnumber (L, 4, 0xffffffff);
    float anchor_x = luaL_optnumber(L, 5, 0.0f);
    float anchor_y = luaL_optnumber(L, 6, 0.0f);
    float font_size = static_cast<float>(luaL_optnumber(L, 7, 16.0f));
    bool monospace = static_cast<bool>(luaL_optinteger(L, 8, 0));
    
    sendMessage(MSGN_LUA_TEXT);
    sendData(&x, sizeof(float));
    sendData(&y, sizeof(float));
    sendString(text);
    sendData(&color, sizeof(uint32_t));
    sendData(&anchor_x, sizeof(float));
    sendData(&anchor_y, sizeof(float));
    sendData(&font_size, sizeof(float));
    sendData(&monospace, sizeof(bool));
    
    return 0;
}

int Lua::Gui::window(lua_State *L)
{
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    std::string id = luaL_checklstring(L, 3, nullptr);
    std::string text = luaL_checklstring(L, 4, nullptr);
    
    sendMessage(MSGN_LUA_WINDOW);
    sendData(&x, sizeof(float));
    sendData(&y, sizeof(float));
    sendString(id);
    sendString(text);
    
    return 0;
}

int Lua::Gui::pixel(lua_State *L)
{
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    uint32_t color = luaL_optnumber (L, 3, 0xffffffff);
    
    sendMessage(MSGN_LUA_PIXEL);
    sendData(&x, sizeof(float));
    sendData(&y, sizeof(float));
    sendData(&color, sizeof(uint32_t));
    
    return 0;
}

int Lua::Gui::rectangle(lua_State *L)
{
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float w = lua_tonumber(L, 3);
    float h = lua_tonumber(L, 4);
    float thickness = luaL_optnumber (L, 5, 1);
    uint32_t color = luaL_optnumber (L, 6, 0xffffffff);
    int filled = luaL_optnumber (L, 7, 0);
    
    sendMessage(MSGN_LUA_RECT);
    sendData(&x, sizeof(float));
    sendData(&y, sizeof(float));
    sendData(&w, sizeof(float));
    sendData(&h, sizeof(float));
    sendData(&thickness, sizeof(float));
    sendData(&color, sizeof(uint32_t));
    sendData(&filled, sizeof(int));
    
    return 0;
}

int Lua::Gui::line(lua_State *L)
{
    float x0 = lua_tonumber(L, 1);
    float y0 = lua_tonumber(L, 2);
    float x1 = lua_tonumber(L, 3);
    float y1 = lua_tonumber(L, 4);
    uint32_t color = luaL_optnumber (L, 5, 0xffffffff);
    
    sendMessage(MSGN_LUA_LINE);
    sendData(&x0, sizeof(float));
    sendData(&y0, sizeof(float));
    sendData(&x1, sizeof(float));
    sendData(&y1, sizeof(float));
    sendData(&color, sizeof(uint32_t));
    
    return 0;
}

int Lua::Gui::quad(lua_State *L)
{
    float x0 = lua_tonumber(L, 1);
    float y0 = lua_tonumber(L, 2);
    float x1 = lua_tonumber(L, 3);
    float y1 = lua_tonumber(L, 4);
    float x2 = lua_tonumber(L, 5);
    float y2 = lua_tonumber(L, 6);
    float x3 = lua_tonumber(L, 7);
    float y3 = lua_tonumber(L, 8);
    float thickness = luaL_optnumber (L, 9, 1);
    uint32_t color = luaL_optnumber (L, 10, 0xffffffff);
    int filled = luaL_optnumber (L, 11, 0);
    
    sendMessage(MSGN_LUA_QUAD);
    sendData(&x0, sizeof(float));
    sendData(&y0, sizeof(float));
    sendData(&x1, sizeof(float));
    sendData(&y1, sizeof(float));
    sendData(&x2, sizeof(float));
    sendData(&y2, sizeof(float));
    sendData(&x3, sizeof(float));
    sendData(&y3, sizeof(float));
    sendData(&thickness, sizeof(float));
    sendData(&color, sizeof(uint32_t));
    sendData(&filled, sizeof(int));
    
    return 0;
}

int Lua::Gui::ellipse(lua_State *L)
{
    float center_x = lua_tonumber(L, 1);
    float center_y = lua_tonumber(L, 2);
    float radius_x = lua_tonumber(L, 3);
    float radius_y = lua_tonumber(L, 4);
    float thickness = luaL_optnumber (L, 5, 1);
    uint32_t color = luaL_optnumber (L, 6, 0xffffffff);
    int filled = luaL_optnumber (L, 7, 0);
    
    sendMessage(MSGN_LUA_ELLIPSE);
    sendData(&center_x, sizeof(float));
    sendData(&center_y, sizeof(float));
    sendData(&radius_x, sizeof(float));
    sendData(&radius_y, sizeof(float));
    sendData(&thickness, sizeof(float));
    sendData(&color, sizeof(uint32_t));
    sendData(&filled, sizeof(int));
    
    return 0;
}
