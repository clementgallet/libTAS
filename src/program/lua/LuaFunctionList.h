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

#ifndef LIBTAS_LUAFUNCTIONLIST_H_INCLUDED
#define LIBTAS_LUAFUNCTIONLIST_H_INCLUDED

#include "NamedLuaFunction.h"

extern "C" {
#include <lua.h>
}
#include <list>
#include <vector>
#include <set>
#include <string>

namespace Lua {

class LuaFunctionList {
    
public:
    
    struct LuaFile {
        lua_State *lua_state;
        std::string file;
        std::string filename;
        int wd;
        bool enabled;
    };
    
    LuaFunctionList();
    ~LuaFunctionList();

    /* Add a named function from lua stack */
    void add(lua_State *L, NamedLuaFunction::CallbackType t);

    /* Import a file */
    void addFile(const std::string& file);

    /* Remove all callbacks from a file */
    void removeForFile(int row);

    /* Look for file changes and reloads them */
    void watchChanges();

    /* Get the active state */
    bool activeState(int row) const;

    /* Activate or deactivate all callbacks from a file */
    void switchForFile(int row, bool active);
    
    /* Call all callbacks from a type */
    void call(NamedLuaFunction::CallbackType c);
    
    /* Returns the number of registered lua files */
    int fileCount() const;
    
    /* Clear all callbacks */
    void clear();

    std::vector<LuaFile> fileList;
    std::set<std::string> fileSet;
    
private:
    std::list<NamedLuaFunction> functions;
    int inotifyfd;

};
}

#endif
