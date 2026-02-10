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

#include "Movie.h"

#include "Context.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static Context* context;

/* List of functions to register */
static const luaL_Reg movie_functions[] =
{
    { "currentFrame", Lua::Movie::currentFrame},
    { "frameCount", Lua::Movie::frameCount},
    { "status", Lua::Movie::status},
    { "time", Lua::Movie::time},
    { "rerecords", Lua::Movie::rerecords},
    { "isDraw", Lua::Movie::isDraw},
    { "getMovieFileName", Lua::Movie::getMovieFileName},
    { "getInitialSystemTime", Lua::Movie::getInitialSystemTime},
    { NULL, NULL }
};

void Lua::Movie::registerFunctions(lua_State *L, Context* c)
{
    context = c;
    luaL_newlib(L, movie_functions);
    lua_setglobal(L, "movie");
}

int Lua::Movie::currentFrame(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(context->framecount));
    return 1;
}

int Lua::Movie::frameCount(lua_State *L)
{
    if (context->config.sc.recording == SharedConfig::NO_RECORDING)
        lua_pushinteger(L, -1);
    else
        lua_pushinteger(L, static_cast<lua_Integer>(context->config.sc.movie_framecount));
    return 1;
}

int Lua::Movie::status(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(context->config.sc.recording));
    return 1;
}

int Lua::Movie::time(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(context->current_time_sec));
    lua_pushinteger(L, static_cast<lua_Integer>(context->current_time_nsec));
    return 2;
}

int Lua::Movie::rerecords(lua_State *L)
{
    if (context->config.sc.recording == SharedConfig::NO_RECORDING)
        lua_pushinteger(L, -1);
    else
        lua_pushinteger(L, static_cast<lua_Integer>(context->rerecord_count));
    return 1;
}

int Lua::Movie::isDraw(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(context->draw_frame));
    return 1;
}

int Lua::Movie::getMovieFileName(lua_State *L)
{
    lua_pushstring(L, context->config.moviefile.c_str());
    return 1;
}

int Lua::Movie::getInitialSystemTime(lua_State *L)
{
    lua_pushinteger(L, context->config.sc.initial_time_sec);
    lua_pushinteger(L, context->config.sc.initial_time_nsec);
    return 2;
}
