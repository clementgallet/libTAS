/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "global.h"

namespace libtas {

/* We use constinit to make sure these globals are initialized at compile time. If not, 
 * the library constructor could be executed before those globals are initialized,
 * resulting in overwriting the globals with their default values. */
constinit SharedConfig Global::shared_config;
constinit GameInfo Global::game_info;
volatile bool Global::is_inited = false;
volatile bool Global::is_exiting = false;
volatile bool Global::is_fork = false;
bool Global::skipping_draw = false;

}
