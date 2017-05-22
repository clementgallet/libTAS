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

#include "DeterministicTimer.h"
#include "NonDeterministicTimer.h"
#include "logging.h"
#include "threads.h"
#include "frame.h"
#include "time.h" // clock_gettime
#include "sleep.h" // nanosleep
#include "audio/AudioContext.h"
#include "ThreadState.h"
#include "renderhud/RenderHUD.h"

struct timespec DeterministicTimer::getTicks()
{
    return getTicks(SharedConfig::TIMETYPE_UNTRACKED);
}

struct timespec DeterministicTimer::getTicks(SharedConfig::TimeCallType type)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    /* If we are in the native thread state, just return the real time */
    if (ThreadState::isNative()) {
        struct timespec realtime;
        clock_gettime(CLOCK_REALTIME, &realtime);
        return realtime;
    }

    if(shared_config.framerate == 0) {
        return nonDetTimer.getTicks(); // 0 framerate means disable deterministic timer
    }

    bool mainT = isMainThread();

    /* If it is our own code calling this, we don't need to track the call */
    if (ThreadState::isOwnCode())
        type = SharedConfig::TIMETYPE_UNTRACKED;

    /* Update the count of time query calls, and advance time if reached the
     * limit. We use different counts for main and secondary threads because
     * advancing time by secondary threads increases the timer with nondeterministic values
     */

    int ticksExtra = 0;

    int gettimes_threshold = mainT ? shared_config.main_gettimes_threshold[type]
                          : shared_config.sec_gettimes_threshold[type];
    if (type != SharedConfig::TIMETYPE_UNTRACKED && gettimes_threshold >= 0) {

        /* We actually track this time call */
        std::lock_guard<std::mutex> lock(mutex);
        debuglog(LCF_TIMESET | LCF_FREQUENT, "subticks ", type, " increased");
        int* gettimes_count = mainT ? &main_gettimes[type] : &sec_gettimes[type];
        (*gettimes_count)++;

        if(*gettimes_count > gettimes_threshold) {
            /*
             * We reached the limit of the number of calls.
             * We advance the deterministic timer by some value
             */
            int tickDelta = 1;

            debuglog(LCF_TIMESET | LCF_FREQUENT, "WARNING! force-advancing time of type ", type);

            ticksExtra += tickDelta;

            /* Reseting the number of calls from all functions */
            for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
                main_gettimes[i] = 0;
                sec_gettimes[i] = 0;
            }
        }
    }

    if(ticksExtra) {
        /* Delay by ticksExtra ms. Arbitrary */
        struct timespec delay = {0, ticksExtra * 1000000};
        addDelay(delay);
    }

    TimeHolder fakeTicks = ticks + fakeExtraTicks;
    return fakeTicks;
}

void DeterministicTimer::addDelay(struct timespec delayTicks)
{
    std::lock_guard<std::mutex> lock(mutex);
    debuglog(LCF_TIMESET | LCF_SLEEP, __func__, " call with delay ", delayTicks.tv_sec * 1000000000 + delayTicks.tv_nsec, " nsec");

    if(shared_config.framerate == 0) // 0 framerate means disable deterministic timer
        return nonDetTimer.addDelay(delayTicks);

    /* We don't handle wait if it is our own code calling this. */
    if (ThreadState::isOwnCode())
        return;

    /* Deferring as much of the delay as possible
     * until the place where the regular per-frame delay is applied
     * gives the smoothest results.
     * However, there must be a limit,
     * otherwise it could easily build up and make us freeze (in some games)
     */

    TimeHolder maxDeferredDelay = timeIncrement * 6;

    addedDelay += delayTicks;
    ticks += delayTicks;
    forceAdvancedTicks += delayTicks;

    if(!shared_config.fastforward)
    {
        /* Sleep, because the caller would have yielded at least a little */
        struct timespec nosleep = {0, 0};
        /* Call the real nanosleep function */
        ThreadNative tn;
        nanosleep(&nosleep, NULL);
    }

    /* We only allow the main thread to trigger a frame boundary! */
    bool mainT = isMainThread();

    if (mainT) {
        while(addedDelay > maxDeferredDelay) {
            /* Indicating that the following frame boundary is not
             * a normal (draw) frame boundary.
             */
            drawFB = false;

            /* We have built up too much delay. We must enter a frame boundary,
             * to advance the time.
             * This decrements addedDelay by (basically) how much it advances ticks
             */
    #ifdef LIBTAS_ENABLE_HUD
            static RenderHUD dummy;
            frameBoundary(false, [] () {}, dummy);
    #else
            frameBoundary(false, [] () {});
    #endif
        }
    }
}

void DeterministicTimer::exitFrameBoundary()
{
    //std::lock_guard<std::mutex> lock(mutex);
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);

    /* Reset the counts of each time get function */
    for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
        main_gettimes[i] = 0;
        sec_gettimes[i] = 0;
    }

    if(shared_config.framerate == 0)
        return nonDetTimer.exitFrameBoundary(); // 0 framerate means disable deterministic timer

    getTimes = 0;

    if(addedDelay > timeIncrement)
        addedDelay -= timeIncrement;
    else {
        addedDelay.tv_sec = 0;
        addedDelay.tv_nsec = 0;
    }
}


void DeterministicTimer::enterFrameBoundary()
{
    //std::lock_guard<std::mutex> lock(mutex);
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);

    if(shared_config.framerate == 0)
        return nonDetTimer.enterFrameBoundary(); // 0 framerate means disable deterministic timer

    /*** First we update the state of the internal timer ***/

    /* We compute by how much we should advance the timer
     * to run exactly as the indicated framerate
     */
    unsigned int integer_increment = 1000000000 / shared_config.framerate;
    unsigned int fractional_increment = 1000000000 % shared_config.framerate;

    timeIncrement.tv_sec = 0;
    timeIncrement.tv_nsec = integer_increment;

    fractional_part += fractional_increment;
    if (fractional_part >= shared_config.framerate)
    {
        timeIncrement.tv_nsec++;
        fractional_part -= shared_config.framerate;
    }

    /* Subtract out ticks that were made when calling GetTicks() */
    /* Warning: adding forceAdvanceTicks or not can yield different results! */
    TimeHolder takenTicks = (ticks - lastEnterTicks) /*+ forceAdvancedTicks*/;

    /* If we enter the frame normally (with a screen draw),
     * then reset the forceAdvancedTicks
     */
    if (drawFB) {
        forceAdvancedTicks.tv_sec = 0;
        forceAdvancedTicks.tv_nsec = 0;
    }

    /*
     * Did we already have advanced more ticks that the length of a frame?
     * If not, we add the remaining ticks
     */
    if (timeIncrement > takenTicks) {
        TimeHolder deltaTicks = timeIncrement - takenTicks;
        ticks += deltaTicks;
        debuglog(LCF_TIMESET | LCF_FRAME, __func__, " added ", deltaTicks.tv_sec * 1000000000 + deltaTicks.tv_nsec, " nsec");
    }

    /* Doing the audio mixing here */
    audiocontext.mixAllSources(timeIncrement);

    /*** Then, we sleep the right amount of time so that the game runs at normal speed ***/

    /* Get the current actual time */
    TimeHolder currentTime;
    {
        ThreadNative tn;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
    }

    /* Calculate the target time we wanted to be at now */
    TimeHolder desiredTime = lastEnterTime + timeIncrement * shared_config.speed_divisor;

    TimeHolder deltaTime = desiredTime - currentTime;

    /* If we are not fast forwarding, and not the first frame,
     * then we wait the delta amount of time.
     */
    if (!shared_config.fastforward && lastEnterValid) {

        /* Check that we wait for a positive time */
        if ((deltaTime.tv_sec > 0) || ((deltaTime.tv_sec == 0) && (deltaTime.tv_nsec >= 0))) {
            /* Call the real nanosleep function */
            ThreadNative tn;
            nanosleep(&deltaTime, NULL);
        }
    }

    /*
     * WARNING: This time update is not done in Hourglass,
     * maybe intentionally (the author does not remember).
     */
    {
        ThreadNative tn;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
    }

    lastEnterTime = currentTime;

    lastEnterTicks = ticks;
    lastEnterValid = true;
}

void DeterministicTimer::fakeAdvanceTimer(struct timespec extraTicks) {
    fakeExtraTicks = extraTicks;
}

void DeterministicTimer::initialize(void)
{
    getTimes = 0;
    ticks.tv_sec = 0;
    ticks.tv_nsec = 0;
    fractional_part = 0;
    {
        ThreadNative tn;
        clock_gettime(CLOCK_MONOTONIC, &lastEnterTime);
    }
    lastEnterTicks = ticks;

    for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
        main_gettimes[i] = 0;
        sec_gettimes[i] = 0;
    }

    addedDelay.tv_sec = 0;
    addedDelay.tv_nsec = 0;
    forceAdvancedTicks = addedDelay;
    fakeExtraTicks = addedDelay;
    lastEnterValid = false;
    drawFB = true;
}

DeterministicTimer detTimer;
