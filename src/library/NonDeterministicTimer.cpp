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

#include "NonDeterministicTimer.h"
#include "logging.h"
#include "frame.h"
#include "global.h" // Global::shared_config
#include "timewrappers.h" // clock_gettime
#include "sleepwrappers.h" // nanosleep
#include "checkpoint/ThreadManager.h"
#include "GlobalState.h"

namespace libtas {

bool NonDeterministicTimer::inited = false;

void NonDeterministicTimer::initialize(void)
{
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lasttime));
    ticks = lasttime;
    lastEnterTicks = ticks;
    inFB = false;
    lastEnterTime = lasttime;
    lastExitTime = lasttime;
    inited = true;
}

struct timespec NonDeterministicTimer::getTicks(void)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    /* If we didn't initialized yet, return a non-zero value. A value of
     * zero breaks some code. */
    if (!inited) {
        return {1, 0};
    }

    /* During a frame boundary, we freeze the timer */
    if (inFB)
        return ticks;

    std::lock_guard<std::mutex> lock(ticks_mutex);

    /* Get the real clock time */
    TimeHolder realtime;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &realtime));

    /* Compute the difference from the last call */
    TimeHolder delta = realtime - lasttime;

    if(Global::shared_config.fastforward) { // fast-forward
        delta = delta * 3; // arbitrary
    }

    /* If we paused at a frame boudary, we should not count that time */
    TimeHolder frameBoundaryDur = lastExitTime - lastEnterTime;

    /* If we just have the game running,
     * do not count the normal frame boundary duration,
     * as it would delay more and more the timer
     */
    if(frameBoundaryDur.tv_sec > 0 || ((frameBoundaryDur.tv_sec >= 0) && (frameBoundaryDur.tv_nsec > 50000000)))
    {
        /* Remove the duration of the frame boundary from the elapsed time */
        delta -= frameBoundaryDur;

        /* Frame duration can only be used once per frame */
        lastEnterTime = lastExitTime;
    }

    ticks += delta;
    debuglogstdio(LCF_TIMESET|LCF_FREQUENT, "%s added %d.%010d", __func__, delta.tv_sec, delta.tv_nsec);

    lasttime = realtime;

    return ticks;
}

TimeHolder NonDeterministicTimer::enterFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET);
    frame_mutex.lock();

    getTicks();
    inFB = true;

    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastEnterTime));

    lastEnterTicks = ticks;

    TimeHolder elapsedTicks = ticks - lastEnterTicks;
    return elapsedTicks;
}

void NonDeterministicTimer::exitFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET);
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastExitTime));
    inFB = false;
    frame_mutex.unlock();
}

void NonDeterministicTimer::addDelay(struct timespec delayTicks)
{
    DEBUGLOGCALL(LCF_SLEEP | LCF_FREQUENT);

    if (Global::shared_config.fastforward) {
        delayTicks.tv_sec = 0;
        delayTicks.tv_nsec = 0;
    }

    /* Call the real nanosleep function */
    NATIVECALL(nanosleep(&delayTicks, NULL));
}

NonDeterministicTimer nonDetTimer;

}
