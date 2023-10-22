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

#include "Main.h"
#include "LuaFunctionList.h"
#include "Gui.h"
#include "Input.h"
#include "Movie.h"
#include "Memory.h"
#include "Print.h"
#include "Callbacks.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

/* Lua state */
// static lua_State *lua_state = nullptr;
static Context* context;

void Lua::Main::init(Context* c)
{
    context = c;
}

lua_State* Lua::Main::new_state()
{    
    lua_State *lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
    
    /* Register our functions */
    Lua::Gui::registerFunctions(lua_state);
    Lua::Input::registerFunctions(lua_state);
    Lua::Memory::registerFunctions(lua_state);
    Lua::Movie::registerFunctions(lua_state, context);
    Lua::Callbacks::registerFunctions(lua_state);
    Lua::Print::init(lua_state);
    
    return lua_state;
}

void Lua::Main::exit()
{
    Lua::Callbacks::clear();
}

std::string luaFile;

int Lua::Main::run(lua_State* lua_state, std::string filename)
{
    luaFile = filename;
    int status = luaL_dofile(lua_state, filename.c_str());
    if (status != 0) {
        std::cerr << "Error " << status << " loading lua script " << filename << std::endl;
        std::cerr << lua_tostring(lua_state, -1) << std::endl;
        lua_pop(lua_state, 1);
        return -1;
    }
    else {
        std::cout << "Loaded script " << filename << std::endl;
    }
    
    /* Push old-style callback methods into new-style */
    lua_getglobal(lua_state, "onStartup");
    if (lua_isfunction(lua_state, -1))
        Lua::Callbacks::onStartup(lua_state);
    else
        lua_pop(lua_state, 1);

    lua_getglobal(lua_state, "onInput");
    if (lua_isfunction(lua_state, -1))
        Lua::Callbacks::onInput(lua_state);
    else
        lua_pop(lua_state, 1);

    lua_getglobal(lua_state, "onFrame");
    if (lua_isfunction(lua_state, -1))
        Lua::Callbacks::onFrame(lua_state);
    else
        lua_pop(lua_state, 1);

    lua_getglobal(lua_state, "onPaint");
    if (lua_isfunction(lua_state, -1))
        Lua::Callbacks::onPaint(lua_state);
    else
        lua_pop(lua_state, 1);

    return 0;
}

const std::string& Lua::Main::currentFile()
{
    return luaFile;
}
