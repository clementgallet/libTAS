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

#include "timewrappers.h"
#include "logging.h"
#include "frame.h"
#include <iomanip> // std::setw
#include "DeterministicTimer.h"
#include "GlobalState.h"
#include "../shared/SharedConfig.h"

namespace libtas {

DEFINE_ORIG_POINTER(clock_gettime)

/* Override */ time_t time(time_t* t) __THROW
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_TIME);
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %d", ts.tv_sec);
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, struct timezone* tz) __THROW
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_GETTIMEOFDAY);
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %d.%06d", ts.tv_sec, ts.tv_nsec/1000);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}

/* Override */ clock_t clock (void) __THROW
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_CLOCK);
    clock_t clk = static_cast<clock_t>(ts.tv_sec) * CLOCKS_PER_SEC + (static_cast<clock_t>(ts.tv_nsec) * CLOCKS_PER_SEC) / 1000000000;
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %d", clk);
    return clk;
}

/* Override */ int clock_gettime (clockid_t clock_id, struct timespec *tp) __THROW
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(clock_gettime);
        return orig::clock_gettime(clock_id, tp);
    }

    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    *tp = detTimer.getTicks(SharedConfig::TIMETYPE_CLOCKGETTIME);
    debuglogstdio(LCF_TIMEGET | LCF_FREQUENT, "  returning %d.%09d", tp->tv_sec, tp->tv_nsec);

    if (shared_config.game_specific_timing & SharedConfig::GC_TIMING_CELESTE) {
        if (ThreadManager::isMainThread())
            detTimer.fakeAdvanceTimer({0, 0});
    }

    return 0;
}

/* Override */ Uint32 SDL_GetTicks(void)
{
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETTICKS);
    Uint32 msec = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    debuglogstdio(LCF_SDL | LCF_TIMEGET, "%s call - returning %d", __func__, msec);

    return msec;
}

/* Override */ Uint64 SDL_GetPerformanceFrequency(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_TIMEGET);
    return 1000000000;
}

/* Override */ Uint64 SDL_GetPerformanceCounter(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_TIMEGET);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER);
    Uint64 counter = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    debuglogstdio(LCF_SDL | LCF_TIMEGET, "  returning %" PRIu64, counter);
    return counter;
}

}
