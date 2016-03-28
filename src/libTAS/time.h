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

#ifndef TIME_H_INCL
#define TIME_H_INCL

#include <time.h>
#include <unistd.h>
#include "../external/SDL.h"
#include "hook.h"
#include "stdbool.h"
#include "global.h"

/* Monotonic system-wide clock.  */
//#   define CLOCK_MONOTONIC      1


//typedef long time_t;
//typedef long suseconds_t;
//typedef unsigned long useconds_t;
/*
# ifndef _STRUCT_TIMEVAL
#  define _STRUCT_TIMEVAL   1
struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};
# endif*/ /* struct timeval */


//int clock_gettime(int clock_id, struct timespec *tp);
//extern int nanosleep (const struct timespec *requested_time,
//                      struct timespec *remaining);

extern unsigned long frame_counter;

extern int (*clock_gettime_real) (clockid_t clock_id, struct timespec *tp);
extern int (*nanosleep_real) (const struct timespec *requested_time, struct timespec *remaining);

OVERRIDE clock_t clock (void);
OVERRIDE int clock_gettime (clockid_t clock_id, struct timespec *tp);
OVERRIDE int usleep(useconds_t usec);
OVERRIDE int nanosleep (const struct timespec *requested_time, struct timespec *remaining);
OVERRIDE time_t time(time_t* t);
OVERRIDE int gettimeofday(struct timeval* tv, struct timezone* tz) throw();
OVERRIDE void SDL_Delay(Uint32 sleep);
OVERRIDE Uint32 SDL_GetTicks(void);
OVERRIDE Uint64 SDL_GetPerformanceFrequency(void);
OVERRIDE Uint64 SDL_GetPerformanceCounter(void);

typedef int SDL_TimerID;
typedef Uint32 (*SDL_NewTimerCallback)(Uint32 interval, void *param);
OVERRIDE SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param);
OVERRIDE SDL_bool SDL_RemoveTimer(SDL_TimerID id);

void link_time(void);
void link_sdltime(void);

#endif
