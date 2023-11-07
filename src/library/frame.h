/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_FRAME_H_INCL
#define LIBTAS_FRAME_H_INCL

#include "renderhud/RenderHUD.h"

#include <functional>
#include <stdint.h>

namespace libtas {

extern uint64_t framecount;

/* Called to initiate a frame boundary.
 * Does several things like:
 * - Advancing timers
 * - Receiving data from libTAS and sending data to it
 * - Dumping audio/video
 *
 * It is mainly called during a screen refresh (when draw() is defined),
 * but can be called also by the timer when we need to advance time to avoid a
 * game softlock (game expect time to pass).
 */
void frameBoundary(std::function<void()> draw, RenderHUD& hud);

}

#endif
