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

#include "monowrappers.h"
#include "sleepwrappers.h" // transfer_sleep()

#include "logging.h"
#include "hookpatch.h"
#include "global.h"
#include "DeterministicTimer.h"
#include "GlobalState.h"
#include "checkpoint/ThreadManager.h" // isMainThread()

namespace libtas {

namespace orig {

unsigned int (*ves_icall_System_Threading_Thread_Sleep_internal)(int ms, void *error);

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

    LOG(LL_TRACE, LCF_SLEEP, "%s call - sleep for %d ms", __func__, ms);

    transfer_sleep(ts, NULL);
}

void hook_mono()
{
    HOOK_PATCH_ORIG(ves_icall_System_Threading_Thread_Sleep_internal, nullptr);    
}

}
