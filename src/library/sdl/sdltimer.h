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

#ifndef LIBTAS_SDLTIMER_H_INCL
#define LIBTAS_SDLTIMER_H_INCL

#include "hook.h"

#include "../external/SDL2.h"

namespace libtas {

/**
 * \brief Add a new timer to the pool of timers already running.
 *
 * \return A timer ID, or 0 when an error occurs.
 */
OVERRIDE sdl2::SDL_TimerID SDL_AddTimer(Uint32 interval, sdl2::SDL_NewTimerCallback callback, void *param);

/**
 * \brief Remove a timer knowing its ID.
 *
 * \return A boolean value indicating success or failure.
 *
 * \warning It is not safe to remove a timer multiple times.
 */
OVERRIDE SDL_bool SDL_RemoveTimer(sdl2::SDL_TimerID id);

}

#endif
