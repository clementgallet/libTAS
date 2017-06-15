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

#ifndef LIBTAS_TIME_H_INCL
#define LIBTAS_TIME_H_INCL

#include <time.h>
#include <unistd.h>
// #include "../external/SDL.h"
#include <SDL2/SDL.h>
#include "hook.h"
// #include <stdbool.h>
#include "global.h"

namespace libtas {

extern unsigned long frame_counter;

/* Time used by the program so far (user time + system time).
   The result / CLOCKS_PER_SECOND is program time in seconds.  */
OVERRIDE clock_t clock (void);

/* Get current value of clock CLOCK_ID and store it in TP.  */
OVERRIDE int clock_gettime (clockid_t clock_id, struct timespec *tp);

/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
OVERRIDE time_t time(time_t* t);

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
OVERRIDE int gettimeofday(struct timeval* tv, struct timezone* tz) throw();

/**
 * \brief Get the number of milliseconds since the SDL library initialization.
 *
 * \note This value wraps if the program runs for more than ~49 days.
 */
OVERRIDE Uint32 SDL_GetTicks(void);

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
