/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "DeterministicTimer.h"
#include "GlobalState.h"
#include "GameHacks.h"
#include "hook.h"
#include "global.h"
#include "checkpoint/ThreadManager.h" // isMainThread()
#include "../shared/SharedConfig.h"

#include <execinfo.h>
#include <iomanip> // std::setw

namespace libtas {

/* Override */ time_t time(time_t* t) __THROW
{
    LOGTRACE(LCF_TIMEGET);
    struct timespec ts = DeterministicTimer::get().getTicks(SharedConfig::TIMETYPE_TIME);
    LOG(LL_DEBUG, LCF_TIMEGET, "  returning %d", ts.tv_sec);
    if (t)
        *t = ts.tv_sec;
    return ts.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, struct timezone* tz) __THROW
{
    LOGTRACE(LCF_TIMEGET);
    struct timespec ts = DeterministicTimer::get().getTicks(SharedConfig::TIMETYPE_GETTIMEOFDAY);
    LOG(LL_DEBUG, LCF_TIMEGET, "  returning %d.%06d", ts.tv_sec, ts.tv_nsec/1000);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}

/* Override */ clock_t clock (void) __THROW
{
    LOGTRACE(LCF_TIMEGET);
    struct timespec ts = DeterministicTimer::get().getTicks(SharedConfig::TIMETYPE_CLOCK);
    clock_t clk = static_cast<clock_t>(ts.tv_sec) * CLOCKS_PER_SEC + (static_cast<clock_t>(ts.tv_nsec) * CLOCKS_PER_SEC) / 1000000000;
    LOG(LL_DEBUG, LCF_TIMEGET, "  returning %d", clk);
    return clk;
}

/* Override */ int clock_gettime (clockid_t clock_id, struct timespec *tp) __THROW
{
    RETURN_IF_NATIVE(clock_gettime, (clock_id, tp), nullptr);

    LOGTRACE(LCF_TIMEGET);

    /* .NET coreclr computes several speed chekcs at startup.
     * Because we don't advance time, it causes a softlock.
     * In this specific case, we advance time to account for this.
     * Getting the caller function is very costly, so we only do it when
     * `libcoreclr.so` has been linked, and on first frame. */
    if (framecount < 4 && GameHacks::hasCoreclr()) {

        /* .NET coreclr computes at startup how much time does a `yield` takes
         * in the Finalizer thread.
         * <https://github.com/dotnet/runtime/blob/140120290aaaffb40e90e8ece982b4f0170c6700/src/coreclr/vm/yieldprocessornormalized.cpp#L46>
         */
        if (GameHacks::getFinalizerThread() == ThreadManager::getThreadTid()) {
            void* return_address =  __builtin_return_address(0);
            char** symbols = backtrace_symbols(&return_address, 1);
            if (symbols != nullptr) {
                if (strstr(symbols[0], "libcoreclr.so")) {
                    LOG(LL_DEBUG, LCF_TIMEGET, "  special advance coreclr yield");
                    struct timespec ts = {0, 1000000};
                    DeterministicTimer::get().addDelay(ts);
                }
                free(symbols);
            }
        }

        /* .NET coreclr computes at startup how much time does process id and TLS take
         * <https://github.com/dotnet/runtime/blob/140120290aaaffb40e90e8ece982b4f0170c6700/src/coreclr/System.Private.CoreLib/src/System/Threading/ProcessorIdCache.cs#L58>
         */
        if (ThreadManager::isMainThread()) {
            void* return_address =  __builtin_return_address(0);
            char** symbols = backtrace_symbols(&return_address, 1);
            if (symbols != nullptr) {
                if (strstr(symbols[0], "libSystem.Native.so")) {
                    LOG(LL_DEBUG, LCF_TIMEGET, "  special advance coreclr TLS");
                    struct timespec ts = {0, 1000000};
                    DeterministicTimer::get().addDelay(ts);
                }
                free(symbols);
            }
        }
    }

    *tp = DeterministicTimer::get().getTicks(DeterministicTimer::get().clockToType(clock_id));
    LOG(LL_DEBUG, LCF_TIMEGET, "  returning %d.%09d", tp->tv_sec, tp->tv_nsec);

    if (Global::shared_config.game_specific_timing & SharedConfig::GC_TIMING_CELESTE) {
        if (ThreadManager::isMainThread())
            DeterministicTimer::get().fakeAdvanceTimer({0, 0});
    }

    return 0;
}

}
