/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_CONFIG_H_INCLUDED
#define LIBTAS_CONFIG_H_INCLUDED

struct Config {
    /* Display frame count in the HUD */
    bool hud_framecount;

    /* Display inputs in the HUD */
    bool hud_inputs;

    /* Do we use our custom memory manager for dynamically allocated memory? */
    bool custom_memorymanager;
    
    /* Prevent the game to write into savefiles */
    bool prevent_savefiles;
};

extern struct Config config;

#endif

