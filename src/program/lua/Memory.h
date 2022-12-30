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

#ifndef LIBTAS_LUAMEMORY_H_INCLUDED
#define LIBTAS_LUAMEMORY_H_INCLUDED

#include <stdint.h>
extern "C" {
#include <lua.h>
}

namespace Lua {

namespace Memory {

    /* Register all functions */
    void registerFunctions(lua_State *L);

    /* Helper function for reading an integer */
    bool read(uintptr_t addr, void* return_value, int size);

    /* Read an unsigned 8-bit integer */
    int readu8(lua_State *L);

    /* Read an unsigned 16-bit integer */
    int readu16(lua_State *L);

    /* Read an unsigned 32-bit integer */
    int readu32(lua_State *L);

    /* Read an unsigned 64-bit integer */
    int readu64(lua_State *L);

    /* Read a signed 8-bit integer */
    int reads8(lua_State *L);

    /* Read a signed 16-bit integer */
    int reads16(lua_State *L);

    /* Read a signed 32-bit integer */
    int reads32(lua_State *L);

    /* Read a signed 64-bit integer */
    int reads64(lua_State *L);

    /* Read a float */
    int readf(lua_State *L);

    /* Read a double */
    int readd(lua_State *L);

    /* Helper function for reading an integer */
    void write(uintptr_t addr, void* value, int size);

    /* Write a 8-bit integer */
    int write8(lua_State *L);

    /* Write a 16-bit integer */
    int write16(lua_State *L);

    /* Write a 32-bit integer */
    int write32(lua_State *L);

    /* Write a 64-bit integer */
    int write64(lua_State *L);
    
    /* Write a float */
    int writef(lua_State *L);
    
    /* Write a double */
    int writed(lua_State *L);
}
}

#endif
