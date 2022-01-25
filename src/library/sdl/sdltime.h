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

#ifndef LIBTAS_SDLTIME_H_INCL
#define LIBTAS_SDLTIME_H_INCL

#include <SDL2/SDL.h>
#include "global.h"

namespace libtas {

/**
 * \brief Get the number of milliseconds since the SDL library initialization.
 *
 * \note This value wraps if the program runs for more than ~49 days.
 */
OVERRIDE Uint32 SDL_GetTicks(void);

/**
 * Get the number of milliseconds since SDL library initialization.
 *
 * \returns an unsigned 64-bit value representing the number of milliseconds
 *          since the SDL library initialized.
 *
 * \since This function is available since SDL 2.0.18.
 */
OVERRIDE Uint64 SDL_GetTicks64(void);

/**
 * \brief Get the count per second of the high resolution counter
 */
OVERRIDE Uint64 SDL_GetPerformanceFrequency(void);

/**
 * \brief Get the current value of the high resolution counter
 */
OVERRIDE Uint64 SDL_GetPerformanceCounter(void);

}

#endif
