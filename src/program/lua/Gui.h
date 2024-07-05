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

#ifndef LIBTAS_LUAGUI_H_INCLUDED
#define LIBTAS_LUAGUI_H_INCLUDED

#include <stdint.h>
extern "C" {
#include <lua.h>
}

namespace Lua {

namespace Gui {

    /* Register all functions */
    void registerFunctions(lua_State *L);

    /* Get the window resolution */
    int resolution(lua_State *L);

    /* Draw text */
    int text(lua_State *L);

    /* Draw window */
    int window(lua_State *L);

    /* Draw pixel */
    int pixel(lua_State *L);

    /* Draw rectangle */
    int rectangle(lua_State *L);

    /* Draw line */
    int line(lua_State *L);

    /* Draw quad */
    int quad(lua_State *L);

    /* Draw ellipse */
    int ellipse(lua_State *L);
}
}

#endif
