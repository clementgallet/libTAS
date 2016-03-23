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
#include "../shared/tasflags.h"
#include "time.h" // clock_gettime_real
#include "audio/AudioBuffer.h"

void NonDeterministicTimer::initialize(void)
{
    ticks.tv_sec = 0;
    ticks.tv_nsec = 0;
    lastEnterTicks = ticks;
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&lasttime);
    frameThreadId = 0;
    lastEnterTime = lasttime;
    lastExitTime = lasttime;
}

struct timespec NonDeterministicTimer::getTicks(void)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    if(frameThreadId == 0)
        frameThreadId = getThreadId();
    /* Only one thread can perform this logic.
     * Could this be a problem...? */
    if(frameThreadId == getThreadId()) {

        /* Get the real clock time */
        TimeHolder realtime;
        clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&realtime);

        /* Compute the difference from the last call */
        TimeHolder delta = realtime - lasttime;

        if(tasflags.fastforward) { // fast-forward
            delta = delta * 3; // arbitrary
        }

        /* If we paused at a frame boudary, we should not count that time */
        TimeHolder frameBoundaryDur = lastExitTime - lastEnterTime;

        /* If we just have the game running,
         * do not count the normal frame boundary duration,
         * as it would delay more and more the timer
         */
        if(frameBoundaryDur.tv_sec > 0 || frameBoundaryDur.tv_nsec > 50000000)
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
    return *(struct timespec*)&ticks;
}

void NonDeterministicTimer::enterFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&lastEnterTime);

    /* Doing the audio mixing here */
    getTicks();
    TimeHolder elapsedTicks = ticks - lastEnterTicks;
    bufferList.mixAllBuffers(*(struct timespec*)&elapsedTicks);
}

void NonDeterministicTimer::exitFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&lastExitTime);
}

void NonDeterministicTimer::addDelay(struct timespec delayTicks)
{
    DEBUGLOGCALL(LCF_SLEEP | LCF_FREQUENT);

    if (tasflags.fastforward) {
        delayTicks.tv_sec = 0;
        delayTicks.tv_nsec = 0;
    }
    
    nanosleep_real(&delayTicks, NULL);
}

NonDeterministicTimer nonDetTimer;

