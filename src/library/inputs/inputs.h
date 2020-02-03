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

#ifndef LIBTAS_INPUTS_H_INCL
#define LIBTAS_INPUTS_H_INCL

/* TODO: I don't know where to put these, so in a separate file for now */

#include "../../shared/AllInputs.h"
#include <X11/X.h>

namespace libtas {

/* Inputs that are sent from libTAS */
extern AllInputs ai;

/* Last state of the inputs */
extern AllInputs old_ai;

/* Fake state of the inputs that is seen by the game.
 * This struct is used when the game want to set inputs, such as
 * Warping the cursor position.
 */
extern AllInputs game_ai;

/* Last state of the game inputs, used to generate events */
extern AllInputs old_game_ai;

/* Same as game_ai but without clipping pointer position */
extern AllInputs game_unclipped_ai;
extern AllInputs old_game_unclipped_ai;

/* Pointer grab window */
extern Window pointer_grab_window;

/* Is the pointer clipped inside a window? */
extern bool pointer_clipping;

/* Coordinates of the clipping window */
extern int clipping_x, clipping_y, clipping_w, clipping_h;

void updateGameInputs();

}

#endif
