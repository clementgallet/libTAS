/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "inputs.h"

namespace libtas {

/* Inputs received from libTAS */
AllInputs ai;

/* Previous inputs, used to detected pressed keys, mouse delta positions, etc. */
AllInputs old_ai;

/*
 * Inputs that are seen by the game. It differs from ai in only rare cases:
 * - when the game warps the mouse cursor
 */
AllInputs game_ai;

}
