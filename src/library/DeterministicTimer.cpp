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

#include "DeterministicTimer.h"
#include "NonDeterministicTimer.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "frame.h"
#include "timewrappers.h" // clock_gettime
#include "sleepwrappers.h" // nanosleep
#include "GlobalState.h"
#include "renderhud/RenderHUD.h"
#include "global.h" // shared_config
#include "BusyLoopDetection.h"

#include <sched.h> // sched_yield()

namespace libtas {

bool DeterministicTimer::inited = false;

struct timespec DeterministicTimer::getTicks()
{
    return getTicks(SharedConfig::TIMETYPE_UNTRACKED);
}

struct timespec DeterministicTimer::getTicks(SharedConfig::TimeCallType type)
{
    /* If we didn't initialized yet, return a non-zero value. A value of
     * zero breaks some code. */
    if (!inited) {
        return {1, 0};
    }

    /* If we are in the native global state, just return the real time */
    if (GlobalState::isNative()) {
        struct timespec realtime;
        clock_gettime(CLOCK_REALTIME, &realtime);
        return realtime;
    }

    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return nonDetTimer.getTicks(); // disable deterministic time
    }

    if ((type == SharedConfig::TIMETYPE_UNTRACKED) || GlobalState::isOwnCode()) {
        TimeHolder fakeTicks = ticks + fakeExtraTicks;
        return fakeTicks;
    }

    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    bool mainT = ThreadManager::isMainThread();

    /* Check for busy loops */
    if (mainT)
        BusyLoopDetection::increment(type);

    /* Update the count of time query calls, and advance time if reached the
     * limit. We use different counts for main and secondary threads because
     * advancing time by secondary threads increases the timer with nondeterministic values
     */

    int ticksExtra = 0;

    int gettimes_threshold = mainT ? shared_config.main_gettimes_threshold[type]
                          : shared_config.sec_gettimes_threshold[type];

    if (!insideFrameBoundary && /* Don't track is already inside a frame boundary */
        gettimes_threshold >= 0) {

        /* We actually track this time call */
        std::lock_guard<std::mutex> lock(ticks_mutex);
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

    if (ticksExtra) {
        /* Delay by ticksExtra ms. Arbitrary */
        struct timespec delay = {0, ticksExtra * 1000000};
        addDelay(delay);
    }

    TimeHolder fakeTicks = ticks + fakeExtraTicks;
    return fakeTicks;
}

void DeterministicTimer::addDelay(struct timespec delayTicks)
{
    debuglog(LCF_TIMESET | LCF_SLEEP, __func__, " call with delay ", delayTicks.tv_sec * 1000000000 + delayTicks.tv_nsec, " nsec");

    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return nonDetTimer.addDelay(delayTicks);

    /* We don't handle wait if it is our own code calling this. */
    if (GlobalState::isOwnCode())
        return;

    /* Deferring as much of the delay as possible
     * until the place where the regular per-frame delay is applied
     * gives the smoothest results.
     * However, there must be a limit,
     * otherwise it could easily build up and make us freeze (in some games)
     */
    {
        std::lock_guard<std::mutex> lock(ticks_mutex);

        addedDelay += delayTicks;
        ticks += delayTicks;
    }

    if(!shared_config.fastforward)
    {
        /* Sleep, because the caller would have yielded at least a little */
        NATIVECALL(sched_yield());
    }

    /* Don't trigger a non-draw frame when game is exiting */
    if (is_exiting)
        return;

    /* We only allow the main thread to trigger a frame boundary! */
    bool mainT = ThreadManager::isMainThread();

    if (mainT && !insideFrameBoundary) {
        TimeHolder maxDeferredDelay = baseTimeIncrement * 2;
        while(addedDelay > maxDeferredDelay) {
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

void DeterministicTimer::flushDelay()
{
    addedDelay = {0, 0};
}

void DeterministicTimer::exitFrameBoundary()
{
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return nonDetTimer.exitFrameBoundary();

    DEBUGLOGCALL(LCF_TIMEGET);

    /* Reset the counts of each time get function */
    for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
        main_gettimes[i] = 0;
        sec_gettimes[i] = 0;
    }

    /* We sleep the right amount of time so that the game runs at normal speed */

    /* Get the current actual time */
    TimeHolder currentTime;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &currentTime));

    /* If we are not fast forwarding, and not the first frame,
     * then we wait the delta amount of time.
     */
    if (!(shared_config.fastforward && (shared_config.fastforward_mode & SharedConfig::FF_SLEEP))) {
        TimeHolder desiredTime = lastEnterTime + baseTimeIncrement * shared_config.speed_divisor;

        /* Call the real nanosleep function */
        NATIVECALL(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &desiredTime, NULL));

        /* We assume that our sleep was perfect, so we save the desired time as our
         * current time, except if our current time was longer than the desired time.
         */
        if (currentTime > desiredTime) {
            lastEnterTime = currentTime;
        }
        else {
            lastEnterTime = desiredTime;
        }
    }

    /* Update baseTimeIncrement if variable framerate */
    if (shared_config.variable_framerate) {
        TimeHolder newTimeIncrement;
        newTimeIncrement.tv_sec = shared_config.framerate_den / shared_config.framerate_num;
        newTimeIncrement.tv_nsec = 1000000000 * (uint64_t)(shared_config.framerate_den % shared_config.framerate_num) / shared_config.framerate_num;

        /* Check if we changed framerate, and reset fractional part if so. */
        if (newTimeIncrement != baseTimeIncrement) {
            baseTimeIncrement = newTimeIncrement;
            fractional_increment = 1000000000 * (uint64_t)(shared_config.framerate_den % shared_config.framerate_num) % shared_config.framerate_num;
            fractional_part = 0;
        }
    }

    insideFrameBoundary = false;
    frame_mutex.unlock();
}


TimeHolder DeterministicTimer::enterFrameBoundary()
{
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return nonDetTimer.enterFrameBoundary();

    frame_mutex.lock();
    DEBUGLOGCALL(LCF_TIMEGET);

    insideFrameBoundary = true;


    /*** First we update the state of the internal timer ***/

    /* We compute by how much we should advance the timer
     * to run exactly as the indicated framerate
     */
    TimeHolder timeIncrement = baseTimeIncrement;

    fractional_part += fractional_increment;
    while (fractional_part >= shared_config.framerate_num)
    {
        timeIncrement.tv_nsec++;
        fractional_part -= shared_config.framerate_num;
    }

    /* If we have less delay than the length of a frame, we advance ticks by
     * the remaining length. Otherwise, we don't increment ticks, and we
     * decrement addedDelay by the time increment.
     */
    if (timeIncrement > addedDelay) {
        TimeHolder deltaTicks = timeIncrement - addedDelay;
        ticks += deltaTicks;
        debuglog(LCF_TIMESET, __func__, " added ", deltaTicks.tv_sec * 1000000000 + deltaTicks.tv_nsec, " nsec");
        addedDelay = {0, 0};
    }
    else {
        addedDelay -= timeIncrement;
    }

    return timeIncrement;
}

void DeterministicTimer::fakeAdvanceTimer(struct timespec extraTicks) {
    fakeExtraTicks = extraTicks;
}

void DeterministicTimer::fakeAdvanceTimerFrame() {
    TimeHolder timeIncrement = baseTimeIncrement;
    unsigned int fake_fractional_part = fractional_part + fractional_increment;
    while (fake_fractional_part >= shared_config.framerate_num)
    {
        timeIncrement.tv_nsec++;
        fake_fractional_part -= shared_config.framerate_num;
    }

    timeIncrement.tv_nsec+=1000000;

    if (timeIncrement > addedDelay) {
        fakeExtraTicks = timeIncrement - addedDelay;
    }
}

void DeterministicTimer::initialize(void)
{
    ticks.tv_sec = shared_config.initial_time_sec;
    ticks.tv_nsec = shared_config.initial_time_nsec;

    if (shared_config.framerate_num > 0) {
        baseTimeIncrement.tv_sec = shared_config.framerate_den / shared_config.framerate_num;
        baseTimeIncrement.tv_nsec = 1000000000 * (uint64_t)(shared_config.framerate_den % shared_config.framerate_num) / shared_config.framerate_num;
        fractional_increment = 1000000000 * (uint64_t)(shared_config.framerate_den % shared_config.framerate_num) % shared_config.framerate_num;
        fractional_part = 0;
    }

    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastEnterTime));

    for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
        main_gettimes[i] = 0;
        sec_gettimes[i] = 0;
    }

    addedDelay = {0, 0};
    fakeExtraTicks = {0, 0};

    inited = true;
}

bool DeterministicTimer::isInsideFrameBoundary()
{
    return insideFrameBoundary;
}

DeterministicTimer detTimer;

}
