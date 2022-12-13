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

#include <sched.h> // sched_yield()

#include "sleepwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "GlobalState.h"
#include "hook.h"
#include "GameHacks.h"
#include <execinfo.h>

namespace libtas {

DEFINE_ORIG_POINTER(nanosleep)
DEFINE_ORIG_POINTER(clock_nanosleep)
DEFINE_ORIG_POINTER(sched_yield)

/* Advance time when sleep call, depending on config and main thread.
 * Returns if the call was transfered.
 */
static bool transfer_sleep(const struct timespec &ts)
{
    if (ts.tv_sec == 0 && ts.tv_nsec == 0)
        return false;

    switch (shared_config.sleep_handling) {
        case SharedConfig::SLEEP_NEVER:
            return false;
        case SharedConfig::SLEEP_MAIN:
            if (!ThreadManager::isMainThread())
                return false;
        case SharedConfig::SLEEP_ALWAYS:
            detTimer.addDelay(ts);
            NATIVECALL(sched_yield());
            return true;        
    }
    return true;
}

/* Override */ void SDL_Delay(unsigned int sleep)
{
    LINK_NAMESPACE_GLOBAL(nanosleep);

    struct timespec ts;
    ts.tv_sec = sleep / 1000;
    ts.tv_nsec = (sleep % 1000) * 1000000;

    if (GlobalState::isNative()) {
        orig::nanosleep(&ts, NULL);
        return;
    }

    debuglogstdio(LCF_SDL | LCF_SLEEP, "%s call - sleep for %d ms", __func__, sleep);

    if (! transfer_sleep(ts))
        orig::nanosleep(&ts, NULL);
}

/* Override */ int usleep(useconds_t usec)
{
    LINK_NAMESPACE_GLOBAL(nanosleep);

    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;

    if (GlobalState::isNative())
        return orig::nanosleep(&ts, NULL);

    bool mainT = ThreadManager::isMainThread();
    debuglogstdio(LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), "%s call - sleep for %d us", __func__, usec);

     /* A bit hackish: Disable sleeps from nvidia driver */
    if (usec && ThreadManager::isMainThread()) {

        void* return_address =  __builtin_return_address(0);
        char** symbols = backtrace_symbols(&return_address, 1);
        if (symbols != nullptr) {
            if (strstr(symbols[0], "libGLX_nvidia.so")) {
                orig::nanosleep(&ts, NULL);
                free(symbols);
                return 0;
            }
            free(symbols);
        }
    }
    
    if (! transfer_sleep(ts))
        orig::nanosleep(&ts, NULL);

    return 0;
}

/* Override */ int nanosleep (const struct timespec *requested_time, struct timespec *remaining)
{
    LINK_NAMESPACE_GLOBAL(nanosleep);

    if (GlobalState::isNative()) {
        return orig::nanosleep(requested_time, remaining);
    }

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d.%09d sec", __func__, requested_time->tv_sec, requested_time->tv_nsec);

    if (! transfer_sleep(*requested_time))
        return orig::nanosleep(requested_time, remaining);

    return 0;
}

/* Override */ int clock_nanosleep (clockid_t clock_id, int flags,
			    const struct timespec *req,
			    struct timespec *rem)
{
    LINK_NAMESPACE_GLOBAL(clock_nanosleep);
    if (GlobalState::isNative()) {
        return orig::clock_nanosleep(clock_id, flags, req, rem);
    }

    TimeHolder sleeptime;
    sleeptime = *req;
    if (flags == 0) {
        /* time is relative */
    }
    else {
        /* time is absolute */
        struct timespec curtime = detTimer.getTicks();
        sleeptime -= curtime;
    }
    
    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d.%09d sec", __func__, sleeptime.tv_sec, sleeptime.tv_nsec);

    if (! transfer_sleep(sleeptime))
        return orig::clock_nanosleep(clock_id, flags, req, rem);

    return 0;
}

/* Override */ int sched_yield(void) __THROW
{
    LINK_NAMESPACE_GLOBAL(sched_yield);

    if (GlobalState::isNative()) {
        return orig::sched_yield();
    }

    DEBUGLOGCALL(LCF_SLEEP);

    if (shared_config.game_specific_timing & SharedConfig::GC_TIMING_CELESTE) {
        if (ThreadManager::isMainThread())
            detTimer.fakeAdvanceTimer({0, 1000000});
    }

    return orig::sched_yield();
}

}
