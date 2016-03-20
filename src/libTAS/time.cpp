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

#include "time.h"
#include "logging.h"
#include "frame.h"
#include "threads.h"
#include <iomanip> // std::setw
#include "DeterministicTimer.h"

/* Frame counter */
unsigned long frame_counter = 0;

/*** Functions that access time ***/
int (*clock_gettime_real) (clockid_t clock_id, struct timespec *tp);

/* Override */ time_t time(time_t* t)
{
    struct timespec ts = detTimer.getTicks(TIMETYPE_TIME);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, __func__, " call - returning ", ts.tv_sec);
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, __attribute__ ((unused)) void* tz)
{
    //struct timespec ts = detTimer.getTicks(TIMETYPE_GETTIMEOFDAY);
    struct timespec ts = detTimer.getTicks(TIMETYPE_UNTRACKED);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, __func__, " call - returning ", ts.tv_sec, ".", std::setw(6), ts.tv_nsec/1000);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}

/* Override */ clock_t clock (void)
{
    struct timespec ts = detTimer.getTicks(TIMETYPE_CLOCK);
    clock_t clk = (clock_t)(((double)ts.tv_sec + (double) (ts.tv_nsec) / 1000000000) * CLOCKS_PER_SEC);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, __func__, " call - returning ", clk);
    return clk;
}

/* Override */ int clock_gettime (clockid_t clock_id, struct timespec *tp)
{
    *tp = detTimer.getTicks(TIMETYPE_CLOCKGETTIME);
    //*tp = detTimer.getTicks(TIMETYPE_UNTRACKED);
    //clock_gettime_real(clock_id, tp);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, __func__, " call, returning ", tp->tv_sec, ".", std::setw(9), tp->tv_nsec);
    return 0;
}

/*** Sleep functions ***/
int (*nanosleep_real) (const struct timespec *requested_time, struct timespec *remaining);

/* Override */ void SDL_Delay(unsigned int sleep)
{
    bool mainT = isMainThread();
    debuglog(LCF_SDL | LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), __func__, " call - sleep for ", sleep, " ms.");

    struct timespec ts;
    ts.tv_sec = sleep / 1000;
    ts.tv_nsec = (sleep % 1000) * 1000000;

    /* If the function was called from the main thread,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (sleep && mainT) {
        detTimer.addDelay(ts);
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
    }

    nanosleep_real(&ts, NULL);
}

/* Override */ int usleep(useconds_t usec)
{
    bool mainT = isMainThread();
    debuglog(LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), __func__, " call - sleep for ", usec, " us.");

    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;

    /* If the function was called from the main thread,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (usec && mainT) {
        detTimer.addDelay(ts);
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
    }

    nanosleep_real(&ts, NULL);
    return 0;
}

/* Override */ int nanosleep (const struct timespec *requested_time, struct timespec *remaining)
{
    bool mainT = isMainThread();
    debuglog(LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), __func__, " call - sleep for ", requested_time->tv_sec * 1000000000 + requested_time->tv_nsec, " nsec");

    /* If the function was called from the main thread,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (mainT) {
        detTimer.addDelay(*requested_time);
        struct timespec owntime = {0, 0};
        return nanosleep_real(&owntime, remaining);
    }

    return nanosleep_real(requested_time, remaining);
}

/* Override */ Uint32 SDL_GetTicks(void)
{
    struct timespec ts = detTimer.getTicks(TIMETYPE_SDLGETTICKS);
    Uint32 msec = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, __func__, " call - returning ", msec);
    
    return msec;
}

/* Override */ Uint64 SDL_GetPerformanceFrequency(void)
{
    //Uint64 freq = SDL_GetPerformanceFrequency_real();
    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, __func__, " call.");
    return 1000000000;
}
/* Override */ Uint64 SDL_GetPerformanceCounter(void)
{
    //Uint64 counter = SDL_GetPerformanceCounter_real();
    
    struct timespec ts = detTimer.getTicks(TIMETYPE_SDLGETPERFORMANCECOUNTER);
    Uint64 counter = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, __func__, " call, returning ", counter);
    return counter;
}

/*** Timers ***/

SDL_TimerID (*SDL_AddTimer_real)(Uint32 interval, SDL_NewTimerCallback callback, void *param);
SDL_bool (*SDL_RemoveTimer_real)(SDL_TimerID id);

/* Override */ SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param)
{
    debuglog(LCF_TIMEFUNC | LCF_SDL | LCF_TODO, "Add SDL Timer with call after ", interval, " ms");
    return SDL_AddTimer_real(interval, callback, param);
}

/* Override */ SDL_bool SDL_RemoveTimer(SDL_TimerID id)
{
    debuglog(LCF_TIMEFUNC | LCF_SDL | LCF_TODO, "Remove SDL Timer.");
    return SDL_RemoveTimer_real(id);
}

void link_time(void)
{
    LINK_SUFFIX(nanosleep, nullptr);
    LINK_SUFFIX(clock_gettime, nullptr);
}

void link_sdltime(void)
{
    if (SDLver == 1) {
        LINK_SUFFIX(SDL_AddTimer, "libSDL-1.2");
        LINK_SUFFIX(SDL_RemoveTimer, "libSDL-1.2");
        /* TODO: Add SDL 1.2 SetTimer */
    }
    if (SDLver == 2) {
        LINK_SUFFIX(SDL_AddTimer, "libSDL2-2");
        LINK_SUFFIX(SDL_RemoveTimer, "libSDL2-2");
    }
}


