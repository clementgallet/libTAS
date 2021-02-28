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

#include "inputs.h"
#include "../global.h"

namespace libtas {

AllInputs ai;
AllInputs old_ai;
AllInputs game_ai;
AllInputs old_game_ai;
AllInputs game_unclipped_ai;
AllInputs old_game_unclipped_ai;

bool pointer_clipping = false;
int clipping_x, clipping_y, clipping_w, clipping_h;

void updateGameInputs()
{
    old_game_ai = game_ai;
    old_game_unclipped_ai = game_unclipped_ai;

    for (int i=0; i<AllInputs::MAXKEYS; i++) {
        game_ai.keyboard[i] = ai.keyboard[i];
    }

    game_ai.pointer_mode = ai.pointer_mode;
    if (game_ai.pointer_mode == SingleInput::POINTER_MODE_RELATIVE) {
        game_ai.pointer_x += ai.pointer_x;
        game_ai.pointer_y += ai.pointer_y;
        game_unclipped_ai.pointer_x += ai.pointer_x;
        game_unclipped_ai.pointer_y += ai.pointer_y;
    }
    else {
        /* If we just switch to absolute, keep the same coords for that frame */
        if (old_game_ai.pointer_mode == SingleInput::POINTER_MODE_RELATIVE) {
            game_ai.pointer_x = old_game_ai.pointer_x;
            game_ai.pointer_y = old_game_ai.pointer_y;
            game_unclipped_ai.pointer_x = game_unclipped_ai.pointer_x;
            game_unclipped_ai.pointer_y = game_unclipped_ai.pointer_y;
        }
        else {
            game_ai.pointer_x += ai.pointer_x - old_ai.pointer_x;
            game_ai.pointer_y += ai.pointer_y - old_ai.pointer_y;
            game_unclipped_ai.pointer_x += ai.pointer_x - old_ai.pointer_x;
            game_unclipped_ai.pointer_y += ai.pointer_y - old_ai.pointer_y;
        }
    }

    game_ai.pointer_mask = ai.pointer_mask;

    for (int ji=0; ji<shared_config.nb_controllers; ji++) {
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            game_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];
        }
        game_ai.controller_buttons[ji] = ai.controller_buttons[ji];
    }

    /* Clipping pointer inside grab window */
    if (pointer_clipping) {
        if (game_ai.pointer_x < clipping_x)
            game_ai.pointer_x = clipping_x;
        else if (game_ai.pointer_x >= (clipping_x + clipping_w))
            game_ai.pointer_x = clipping_x + clipping_w - 1;

        if (game_ai.pointer_y < clipping_y)
            game_ai.pointer_y = clipping_y;
        else if (game_ai.pointer_y >= (clipping_y + clipping_h))
            game_ai.pointer_y = clipping_y + clipping_h - 1;
    }

    old_ai = ai;
}

}
