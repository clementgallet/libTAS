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

#include "Main.h"
#include "Gui.h"
#include "Input.h"
#include "Movie.h"
#include "Memory.h"
#include <iostream>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

void Lua::Main::init(Context* context)
{
    if (context->lua_state)
        lua_close(context->lua_state);
    
    context->lua_state = luaL_newstate();
    luaL_openlibs(context->lua_state);
    
    /* Register our functions */
    Lua::Gui::registerFunctions(context);
    Lua::Input::registerFunctions(context);
    Lua::Memory::registerFunctions(context);
    Lua::Movie::registerFunctions(context);
}

void Lua::Main::exit(Context* context)
{
    if (context->lua_state)
        lua_close(context->lua_state);
    context->lua_state = nullptr;
}

void Lua::Main::run(Context* context, std::string filename)
{
    int status = luaL_dofile(context->lua_state, filename.c_str());
    if (status != 0) {
        std::cerr << "Error " << status << " loading lua script " << filename << std::endl;
        std::cerr << lua_tostring(context->lua_state, -1) << std::endl;
    }
    else {
        std::cout << "Loaded script " << filename << std::endl;        
    }
}

void Lua::Main::reset(Context* context)
{
    exit(context);
    init(context);
}

void Lua::Main::callLua(Context* context, const char* func)
{
    if (!context->lua_state) return;
    
    lua_getglobal(context->lua_state, func);
    if (lua_isfunction(context->lua_state, -1)) {
        int ret = lua_pcall(context->lua_state, 0, 0, 0);
        if (ret != 0) {
            std::cerr << "error running function "<< func << "(): " << lua_tostring(context->lua_state, -1) << std::endl;
            lua_pop(context->lua_state, 1);  // pop error message from the stack
        }
    }
    else {
        /* No function, we need to clear the stack */
        lua_pop(context->lua_state, 1);
    }
}
