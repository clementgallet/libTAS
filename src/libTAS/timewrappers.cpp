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

#include "timewrappers.h"
#include "logging.h"
#include "frame.h"
#include "threadwrappers.h"
#include <iomanip> // std::setw
#include "DeterministicTimer.h"
// #include "backtrace.h"
#include "GlobalState.h"
#include "../shared/SharedConfig.h"

namespace libtas {

/* Frame counter */
unsigned long frame_counter = 0;

DEFINE_ORIG_POINTER(clock_gettime);

/* Override */ time_t time(time_t* t) throw()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_TIME);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", ts.tv_sec);
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, struct timezone* tz) throw()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_GETTIMEOFDAY);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", ts.tv_sec, ".", std::setw(6), ts.tv_nsec/1000);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}

/* Override */ clock_t clock (void) throw()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_CLOCK);
    clock_t clk = static_cast<clock_t>((static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1000000000.0) * CLOCKS_PER_SEC);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", clk);
    return clk;
}

/* Override */ int clock_gettime (clockid_t clock_id, struct timespec *tp) throw()
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(clock_gettime, nullptr);
        return orig::clock_gettime(clock_id, tp);
    }

    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);
    *tp = detTimer.getTicks(SharedConfig::TIMETYPE_CLOCKGETTIME);
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "  returning ", tp->tv_sec, ".", std::setw(9), tp->tv_nsec);
    return 0;
}

/* Override */ Uint32 SDL_GetTicks(void)
{
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETTICKS);
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
    struct timespec ts = detTimer.getTicks(SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER);
    Uint64 counter = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, "  returning ", counter);
    return counter;
}

}
