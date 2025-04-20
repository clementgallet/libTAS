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

#include "Memory.h"

#include "ramsearch/MemAccess.h"
#include "ramsearch/BaseAddresses.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

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
    { "readf", Lua::Memory::readf},
    { "readd", Lua::Memory::readd},
    { "readcstring", Lua::Memory::readcstring},
    { "write8", Lua::Memory::write8},
    { "write16", Lua::Memory::write16},
    { "write32", Lua::Memory::write32},
    { "write64", Lua::Memory::write64},
    { "writef", Lua::Memory::writef},
    { "writed", Lua::Memory::writed},
    { "baseAddress", Lua::Memory::baseAddress},
    { NULL, NULL }
};

void Lua::Memory::registerFunctions(lua_State *L)
{
    luaL_newlib(L, memory_functions);
    lua_setglobal(L, "memory");
}

bool Lua::Memory::read(uintptr_t addr, void* return_value, int size)
{
    return MemAccess::read(return_value, reinterpret_cast<void*>(addr), size) == (size_t)size;
}

/* Define a macro to declare all read functions */
#define READFUNCINT(NAME, TYPE) \
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

READFUNCINT(u8, uint8_t)
READFUNCINT(u16, uint16_t)
READFUNCINT(u32, uint32_t)
READFUNCINT(u64, uint64_t)
READFUNCINT(s8, int8_t)
READFUNCINT(s16, int16_t)
READFUNCINT(s32, int32_t)
READFUNCINT(s64, int64_t)

#define READFUNCNUMBER(NAME, TYPE) \
int Lua::Memory::read##NAME(lua_State *L) \
{ \
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1)); \
    TYPE value; \
    bool ret = read(addr, &value, sizeof(TYPE)); \
    if (ret) \
        lua_pushnumber(L, static_cast<lua_Number>(value)); \
    else \
        lua_pushnumber(L, static_cast<lua_Number>(0)); \
    return 1; \
}

READFUNCNUMBER(f, float)
READFUNCNUMBER(d, double)

int Lua::Memory::readcstring(lua_State *L)
{
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1));
    int max_length = lua_tointeger(L, 2);
    
    char* buf = new char[max_length]{};
    MemAccess::read(buf, reinterpret_cast<void*>(addr), max_length);
    buf[max_length-1] = '\0';
    lua_pushstring(L, buf);
    delete[] buf;
    return 1;
}

void Lua::Memory::write(uintptr_t addr, void* value, int size)
{
    MemAccess::write(value, reinterpret_cast<void*>(addr), size);
}

/* Define a macro to declare all write functions */
#define WRITEFUNCINT(NAME, TYPEU, TYPES) \
int Lua::Memory::write##NAME(lua_State *L) \
{ \
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1)); \
    lua_Integer value = lua_tointeger(L, 2); \
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

WRITEFUNCINT(8, uint8_t, int8_t)
WRITEFUNCINT(16, uint16_t, int16_t)
WRITEFUNCINT(32, uint32_t, int32_t)
WRITEFUNCINT(64, uint64_t, int64_t)

#define WRITEFUNCNUMBER(NAME, TYPE) \
int Lua::Memory::write##NAME(lua_State *L) \
{ \
    uintptr_t addr = static_cast<uintptr_t>(lua_tointeger(L, 1)); \
    lua_Number value = lua_tonumber(L, 2); \
    TYPE typed_value = static_cast<TYPE>(value); \
    write(addr, &typed_value, sizeof(TYPE)); \
    return 0; \
}

WRITEFUNCNUMBER(f, float)
WRITEFUNCNUMBER(d, double)

int Lua::Memory::baseAddress(lua_State *L)
{
    std::string text = luaL_checklstring(L, 1, nullptr);
    uintptr_t addr = BaseAddresses::getBaseAddress(text);
    lua_pushinteger(L, static_cast<lua_Integer>(addr));
    return 1;
}
