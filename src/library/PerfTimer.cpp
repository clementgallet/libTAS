/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "PerfTimer.h"
#include "logging.h"
#include "GlobalState.h"
#include "checkpoint/ThreadManager.h"

#include <time.h>

namespace libtas {

void PerfTimer::switchTimer(TimerType type)
{
    /* Stop and increase old timer */
    if (current_type != NoTimer) {
        elapsed[current_type] += (TimeHolder::now() - current_time[current_type]);
    }
    
    /* Start new timer */
    current_type = type;
    current_time[current_type] = TimeHolder::now();
}

PerfTimer::TimerType PerfTimer::currentTimer()
{
    return current_type;
}

void PerfTimer::print()
{
    LOG(LL_INFO, LCF_NONE, "Game timer took %d.%03d sec", elapsed[GameTimer].tv_sec, elapsed[GameTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Frame timer took %d.%03d sec", elapsed[FrameTimer].tv_sec, elapsed[FrameTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Render timer took %d.%03d sec", elapsed[RenderTimer].tv_sec, elapsed[RenderTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Idle timer took %d.%03d sec", elapsed[IdleTimer].tv_sec, elapsed[IdleTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Wait timer took %d.%03d sec", elapsed[WaitTimer].tv_sec, elapsed[WaitTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Time timer took %d.%03d sec", elapsed[TimeTimer].tv_sec, elapsed[TimeTimer].tv_nsec / 1000000);
    LOG(LL_INFO, LCF_NONE, "Special timer took %d.%03d sec", elapsed[SpecialTimer].tv_sec, elapsed[SpecialTimer].tv_nsec / 1000000);

    for (int i=0; i < TotalTimer; i++) {
        elapsed[i].tv_sec = 0;
        elapsed[i].tv_nsec = 0;
    }
}

PerfTimerCall::PerfTimerCall(LogCategoryFlag lcf)
{
    if ((lcf & LCF_TIMEGET) && ThreadManager::isMainThread() && perfTimer.currentTimer() == PerfTimer::GameTimer) {
        type = PerfTimer::TimeTimer;
        perfTimer.switchTimer(PerfTimer::TimeTimer);
    }
}

PerfTimerCall::~PerfTimerCall()
{
    if (perfTimer.currentTimer() == type)
        perfTimer.switchTimer(PerfTimer::GameTimer);
}

PerfTimer perfTimer;

}
