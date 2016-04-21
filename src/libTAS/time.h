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
#include "../external/SDL.h"
#include "hook.h"
#include "stdbool.h"
#include "global.h"

extern unsigned long frame_counter;

namespace orig {
    /* We need at least one function to get the real clock time */
    extern int (*clock_gettime) (clockid_t clock_id, struct timespec *tp);

    /* We need at least one function to make a sleep */
    extern int (*nanosleep) (const struct timespec *requested_time, struct timespec *remaining);
}

/* Time used by the program so far (user time + system time).
   The result / CLOCKS_PER_SECOND is program time in seconds.  */
OVERRIDE clock_t clock (void);

/* Get current value of clock CLOCK_ID and store it in TP.  */
OVERRIDE int clock_gettime (clockid_t clock_id, struct timespec *tp);

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

/* Return the current time and put it in *TIMER if TIMER is not NULL.  */
OVERRIDE time_t time(time_t* t);

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
OVERRIDE int gettimeofday(struct timeval* tv, struct timezone* tz) throw();

/**
 * \brief Wait a specified number of milliseconds before returning.
 */
OVERRIDE void SDL_Delay(Uint32 sleep);

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

typedef int SDL_TimerID;
typedef Uint32 (*SDL_NewTimerCallback)(Uint32 interval, void *param);

/**
 * \brief Add a new timer to the pool of timers already running.
 *
 * \return A timer ID, or 0 when an error occurs.
 */
OVERRIDE SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param);

/**
 * \brief Remove a timer knowing its ID.
 *
 * \return A boolean value indicating success or failure.
 *
 * \warning It is not safe to remove a timer multiple times.
 */
OVERRIDE SDL_bool SDL_RemoveTimer(SDL_TimerID id);

void link_time(void);
void link_sdltime(void);

#endif
