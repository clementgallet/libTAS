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

#ifndef LIBTAS_LUAMAIN_H_INCLUDED
#define LIBTAS_LUAMAIN_H_INCLUDED

#include "Context.h"

#include <string>

struct lua_State;

namespace Lua {

namespace Main {

    /* Init */
    void init(Context* context);
    
    /* Create and initialize a new lua state */
    lua_State* new_state();

    /* Exit */
    void exit();

    /* Run a lua script */
    int run(lua_State* lua_state, std::string filename);

    /* Return the current executed filename */
    const std::string& currentFile();
}
}

#endif
