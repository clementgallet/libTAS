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

#include "monowrappers.h"
// #include <cstdlib>

#include "logging.h"
#include "hookpatch.h"
#include "global.h"
#include "DeterministicTimer.h"

namespace libtas {

/* Advance time when sleep call, depending on config and main thread.
 * Returns if the call was transfered.
 * TODO: duplicate of sleepwrappers.cpp
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

namespace orig {

static unsigned int __attribute__((noinline)) ves_icall_System_Threading_Thread_Sleep_internal(int ms, void *error)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

}

void ves_icall_System_Threading_Thread_Sleep_internal(int ms, void *error)
{
        
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    if (GlobalState::isNative()) {
        orig::ves_icall_System_Threading_Thread_Sleep_internal(ms, error);
        return;
    }

    if (ms == 0)
        return;

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d ms", __func__, ms);

    if (! transfer_sleep(ts))
        orig::ves_icall_System_Threading_Thread_Sleep_internal(ms, error);
}

void hook_mono()
{
    HOOK_PATCH_ORIG(ves_icall_System_Threading_Thread_Sleep_internal, nullptr);
}

}
