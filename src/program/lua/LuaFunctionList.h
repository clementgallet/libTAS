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

#ifndef LIBTAS_LUAFUNCTIONLIST_H_INCLUDED
#define LIBTAS_LUAFUNCTIONLIST_H_INCLUDED

#include "NamedLuaFunction.h"

extern "C" {
#include <lua.h>
}
#include <list>
#include <string>

namespace Lua {

class LuaFunctionList {
    
public:
    /* Add a named function from lua stack */
    void add(lua_State *L, NamedLuaFunction::CallbackType t);

    /* Remove all callbacks from a file */
    void removeForFile(const std::string& file);
    
    /* Call all callbacks from a type */
    void call(NamedLuaFunction::CallbackType c);
    
    /* Clear all callbacks */
    void clear();
    
private:
    std::list<NamedLuaFunction> functions;
    
};
}

#endif
