/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SLEEP_H_INCL
#define LIBTAS_SLEEP_H_INCL

#include "hook.h"

#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>

namespace libtas {

/* Advance time when sleep call, depending on config and main thread.
 * Returns if the call was transfered.
 */
void transfer_sleep(const struct timespec &ts, struct timespec *rem);

/* Sleep USECONDS microseconds, or until a signal arrives that is not blocked
   or ignored.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int usleep(useconds_t usec);

/* Pause execution for a number of nanoseconds.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int nanosleep (const struct timespec *requested_time, struct timespec *remaining);

/* High-resolution sleep with the specified clock. */
OVERRIDE int clock_nanosleep (clockid_t clock_id, int flags,
			    const struct timespec *req,
			    struct timespec *rem);

/**
 * \brief Wait a specified number of milliseconds before returning.
 */
OVERRIDE void SDL_Delay(Uint32 sleep);

OVERRIDE int sched_yield(void) __THROW;

}

#endif
