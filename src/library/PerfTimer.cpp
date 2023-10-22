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

#include "PerfTimer.h"
#include "logging.h"
#include "GlobalState.h"

#include <time.h>

namespace libtas {

void PerfTimer::switchTimer(TimerType type)
{
    /* Stop and increase old timer */
    if (current_type != -1) {
        TimeHolder end_time;
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &end_time));
        
        elapsed[current_type] += (end_time - current_time[current_type]);
    }
    
    /* Start new timer */
    current_type = type;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time[current_type]));
}

void PerfTimer::print()
{
    debuglogstdio(LCF_INFO, "Game timer took %d.%03d sec", elapsed[GameTimer].tv_sec, elapsed[GameTimer].tv_nsec / 1000000);
    debuglogstdio(LCF_INFO, "Frame timer took %d.%03d sec", elapsed[FrameTimer].tv_sec, elapsed[FrameTimer].tv_nsec / 1000000);
    debuglogstdio(LCF_INFO, "Render timer took %d.%03d sec", elapsed[RenderTimer].tv_sec, elapsed[RenderTimer].tv_nsec / 1000000);
    debuglogstdio(LCF_INFO, "Idle timer took %d.%03d sec", elapsed[IdleTimer].tv_sec, elapsed[IdleTimer].tv_nsec / 1000000);
}

PerfTimer perfTimer;

}
