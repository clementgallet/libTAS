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

#include "NonDeterministicTimer.h"
#include "logging.h"
#include "frame.h"
#include "global.h" // shared_config
#include "timewrappers.h" // clock_gettime
#include "sleepwrappers.h" // nanosleep
#include "checkpoint/ThreadManager.h"
#include "audio/AudioContext.h"

namespace libtas {

void NonDeterministicTimer::initialize(void)
{
    ticks.tv_sec = 0;
    ticks.tv_nsec = 0;
    lastEnterTicks = ticks;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lasttime));
    inFB = false;
    lastEnterTime = lasttime;
    lastExitTime = lasttime;
}

struct timespec NonDeterministicTimer::getTicks(void)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    /* During a frame boundary, we freeze the timer */
    if (inFB)
        return ticks;

    bool isFrameThread = ThreadManager::isMainThread();

    /* Only the main thread can modify the timer */
    /* Could this be a problem...? */
    if(isFrameThread) {

        /* Get the real clock time */
        TimeHolder realtime;
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &realtime));

        /* Compute the difference from the last call */
        TimeHolder delta = realtime - lasttime;

        if(shared_config.fastforward) { // fast-forward
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
        debuglog(LCF_TIMESET|LCF_FREQUENT, __func__, " added ", delta.tv_sec * 1000000000 + delta.tv_nsec, " nsec ");

        lasttime = realtime;
    }
    return ticks;
}

void NonDeterministicTimer::enterFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);
    getTicks();
    inFB = true;

    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastEnterTime));

    /* Doing the audio mixing here */
    TimeHolder elapsedTicks = ticks - lastEnterTicks;
    audiocontext.mixAllSources(elapsedTicks);

    lastEnterTicks = ticks;
}

void NonDeterministicTimer::exitFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastExitTime));
    inFB = false;
}

void NonDeterministicTimer::addDelay(struct timespec delayTicks)
{
    DEBUGLOGCALL(LCF_SLEEP | LCF_FREQUENT);

    if (shared_config.fastforward) {
        delayTicks.tv_sec = 0;
        delayTicks.tv_nsec = 0;
    }

    /* Call the real nanosleep function */
    NATIVECALL(nanosleep(&delayTicks, NULL));
}

NonDeterministicTimer nonDetTimer;

}
