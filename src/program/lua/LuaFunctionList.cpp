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

#include "LuaFunctionList.h"

namespace Lua {

void LuaFunctionList::add(lua_State *L, NamedLuaFunction::CallbackType t)
{
    functions.emplace_back(L, t);
}

void LuaFunctionList::removeForFile(const std::string& file)
{
    functions.remove_if([&file](const NamedLuaFunction& nlf){ return 0 == file.compare(nlf.file); });
}

void LuaFunctionList::call(NamedLuaFunction::CallbackType c)
{
    for (auto& nlf : functions) {
        if (nlf.type == c) {
            nlf.call();
        }
    }
}

void LuaFunctionList::clear()
{
    functions.clear();
}

}
