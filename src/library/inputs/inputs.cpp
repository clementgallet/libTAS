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
#include "../global.h"

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

void updateGameInputs()
{
    for (int i=0; i<AllInputs::MAXKEYS; i++) {
        game_ai.keyboard[i] = ai.keyboard[i];
    }

    game_ai.pointer_x += ai.pointer_x - old_ai.pointer_x;
    game_ai.pointer_y += ai.pointer_y - old_ai.pointer_y;
    game_ai.pointer_mask = ai.pointer_mask;

    for (int ji=0; ji<shared_config.nb_controllers; ji++) {
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            game_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];
        }
        game_ai.controller_buttons[ji] = ai.controller_buttons[ji];
    }

}

}
