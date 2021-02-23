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

#include "Memory.h"
#include "../ramsearch/MemAccess.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static Context* context;

/* List of functions to register */
static const luaL_Reg memory_functions[] =
{
    { "readu8", Lua::Memory::readu8},
    { "readu16", Lua::Memory::readu16},
    { "readu32", Lua::Memory::readu32},
    { "readu64", Lua::Memory::readu64},
    { "reads8", Lua::Memory::reads8},
    { "reads16", Lua::Memory::reads16},
    { "reads32", Lua::Memory::reads32},
    { "reads64", Lua::Memory::reads64},
    { "write8", Lua::Memory::write8},
    { "write16", Lua::Memory::write16},
    { "write32", Lua::Memory::write32},
    { "write64", Lua::Memory::write64},
    { NULL, NULL }
};

void Lua::Memory::registerFunctions(Context* c)
{
    context = c;
    luaL_newlib(context->lua_state, memory_functions);
    lua_setglobal(context->lua_state, "memory");
}

bool Lua::Memory::read(uintptr_t addr, void* return_value, int size)
{
    return MemAccess::read(return_value, reinterpret_cast<void*>(addr), size) == size;
}

/* Define a macro to declare all read functions */
#define READFUNC(NAME, TYPE) \
int Lua::Memory::read##NAME(lua_State *L) \
{ \
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1)); \
    TYPE value; \
    bool ret = read(addr, &value, sizeof(TYPE)); \
    if (ret) \
        lua_pushinteger(L, static_cast<lua_Integer>(value)); \
    else \
        lua_pushinteger(L, static_cast<lua_Integer>(0)); \
    return 1; \
}

READFUNC(u8, uint8_t)
READFUNC(u16, uint16_t)
READFUNC(u32, uint32_t)
READFUNC(u64, uint64_t)
READFUNC(s8, int8_t)
READFUNC(s16, int16_t)
READFUNC(s32, int32_t)
READFUNC(s64, int64_t)

void Lua::Memory::write(uintptr_t addr, void* value, int size)
{
    MemAccess::write(value, reinterpret_cast<void*>(addr), size);
}

/* Define a macro to declare all write functions */
#define WRITEFUNC(NAME, TYPEU, TYPES) \
int Lua::Memory::write##NAME(lua_State *L) \
{ \
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1)); \
    int64_t value = static_cast<int64_t>(lua_tointeger(L, 2)); \
    if (value >= 0) { \
        TYPEU typed_value = static_cast<TYPEU>(value); \
        write(addr, &typed_value, sizeof(TYPEU)); \
    } \
    else { \
        TYPES typed_value = static_cast<TYPES>(value); \
        write(addr, &typed_value, sizeof(TYPES)); \
    } \
    return 0; \
}

WRITEFUNC(8, uint8_t, int8_t)
WRITEFUNC(16, uint16_t, int16_t)
WRITEFUNC(32, uint32_t, int32_t)
WRITEFUNC(64, uint64_t, int64_t)
