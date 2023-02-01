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

#include "Print.h"

#include <sstream>
extern "C" {
#include <lua.h>
// #include <lualib.h>
#include <lauxlib.h>
}

void Lua::Print::init(lua_State *L) {
    lua_pushcfunction(L, print);
    lua_setglobal(L, "print");
}

/* Redefine lua `print` function to output on a Qt window */
int Lua::Print::print(lua_State *L) {
    std::ostringstream oss;
    int nargs = lua_gettop(L);
    for (int i = 1; i <= nargs; ++i) {
        oss << luaL_tolstring(L, i, nullptr);
        lua_pop(L, 1); // remove the string
    }
    
    QString qstr(oss.str().c_str());
    
    emit get().signalPrint(qstr);
    return 0;
}
