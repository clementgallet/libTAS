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
#include "Main.h"
#include "../utils.h"

namespace Lua {

void LuaFunctionList::add(lua_State *L, NamedLuaFunction::CallbackType t)
{
    functions.emplace_back(L, t);
}

void LuaFunctionList::addFile(const std::string& file)
{
    if (fileSet.find(file) != fileSet.end())
        return;
        
    fileSet.insert(file);
    fileList.push_back(file);
    fileNameList.push_back(fileFromPath(file));
    fileEnabled.push_back(true);
    Lua::Main::run(file);
}

void LuaFunctionList::removeForFile(int row)
{
    const std::string& file = fileList[row];    
    functions.remove_if([&file](const NamedLuaFunction& nlf){ return 0 == file.compare(nlf.file); });
    fileSet.erase(file);
    fileList.erase(fileList.begin() + row);
    fileNameList.erase(fileNameList.begin() + row);
    fileEnabled.erase(fileEnabled.begin() + row);
}

bool LuaFunctionList::activeState(int row) const
{
    return fileEnabled[row];
}

void LuaFunctionList::switchForFile(int row, bool active)
{
    fileEnabled[row] = active;
    const std::string& file = fileList[row];
    for (auto& nlf : functions) {
        if (0 == file.compare(nlf.file)) {
            nlf.active = active;
        }
    }
}

void LuaFunctionList::call(NamedLuaFunction::CallbackType c)
{
    for (auto& nlf : functions) {
        if (nlf.active && nlf.type == c) {
            nlf.call();
        }
    }
}

int LuaFunctionList::fileCount() const
{
    return fileSet.size();
}

void LuaFunctionList::clear()
{
    functions.clear();
    fileList.clear();
    fileNameList.clear();
    fileEnabled.clear();
    fileSet.clear();
}

}
