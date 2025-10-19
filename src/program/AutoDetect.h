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

#ifndef LIBTAS_AUTODETECT_H_INCLUDED
#define LIBTAS_AUTODETECT_H_INCLUDED

/* Forward declaration */
class MovieFile;
struct Context;

namespace AutoDetect {
    
    enum Engine {
        ENGINE_UNITY,
        ENGINE_GAMEMAKER,
        ENGINE_GODOT,
        ENGINE_UNKNOWN,
    };
    
    /* Detect and returns the game executable arch */
    int arch(Context *context);

    /* Auto-detect the local directory containing the bundle libraries shipped
     * with the game, so that in most cases users don't have to specify it. */
    void game_libraries(Context *context);

    /* Auto-detect game engine and apply settings to common engines */
    int game_engine(Context *context);

}

#endif
