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
#include "backtrace.h"
#include "ThreadState.h"

/* Frame counter */
unsigned long frame_counter = 0;

/*** Functions that access time ***/
int (*clock_gettime_real) (clockid_t clock_id, struct timespec *tp);

/* Override */ time_t time(time_t* t)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(TIMETYPE_TIME);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", ts.tv_sec);
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, struct timezone* tz) throw()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(TIMETYPE_UNTRACKED);
    //struct timespec ts = detTimer.getTicks(TIMETYPE_GETTIMEOFDAY);

    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", ts.tv_sec, ".", std::setw(6), ts.tv_nsec/1000);
    //printBacktrace();
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}

/* Override */ clock_t clock (void)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(TIMETYPE_CLOCK);
    clock_t clk = (clock_t)(((double)ts.tv_sec + (double) (ts.tv_nsec) / 1000000000) * CLOCKS_PER_SEC);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", clk);
    return clk;
}

/* Override */ int clock_gettime (clockid_t clock_id, struct timespec *tp)
{
    if (!clock_gettime_real) {
        /* Some libraries can call this *very* early, so trying to link here */
        link_time();
        if (!clock_gettime_real) {
            tp->tv_sec = 0;
            tp->tv_nsec = 0;
            return 0;
        }
    }
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    if (threadState.isNative()) {
        clock_gettime_real(clock_id, tp);
    }
    else {
        //*tp = detTimer.getTicks(TIMETYPE_UNTRACKED);
        *tp = detTimer.getTicks(TIMETYPE_CLOCKGETTIME);
    }
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", tp->tv_sec, ".", std::setw(9), tp->tv_nsec);
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

    /* If the function was called from the main thread
     * and we are not in the native thread state,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (sleep && mainT && !threadState.isNative()) {
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

    /* If the function was called from the main thread
     * and we are not in the native thread state,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (usec && mainT && !threadState.isNative()) {
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

    /* If the function was called from the main thread
     * and we are not in the native thread state,
     * transfer the wait to the timer and
     * do not actually wait
     */
    if (mainT && !threadState.isNative()) {
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
    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, __func__, " call.");
    return 1000000000;
}

/* Override */ Uint64 SDL_GetPerformanceCounter(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_TIMEGET | LCF_FRAME);
    struct timespec ts = detTimer.getTicks(TIMETYPE_SDLGETPERFORMANCECOUNTER);
    Uint64 counter = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, "  returning ", counter);
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
    LINK_SUFFIX_SDLX(SDL_AddTimer);
    LINK_SUFFIX_SDLX(SDL_RemoveTimer);
    /* TODO: Add SDL 1.2 SetTimer */
}


