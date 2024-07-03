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

#include "DeterministicTimer.h"
#include "NonDeterministicTimer.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "frame.h"
#include "general/timewrappers.h" // clock_gettime
#include "general/sleepwrappers.h" // nanosleep
#include "GlobalState.h"
#include "renderhud/RenderHUD.h"
#include "global.h" // Global::shared_config
#include "BusyLoopDetection.h"
#include "PerfTimer.h"

#include <sched.h> // sched_yield()
#include <stdint.h>

/* Number of time call to generate a warning */
#define ALERT_CALL_THRESHOLD 100000

namespace libtas {

bool DeterministicTimer::inited = false;

const char* const DeterministicTimer::gettimes_names[] = 
{
    "time()",
    "gettimeofday()",
    "clock()",
    "clock_gettime(CLOCK_REALTIME)",
    "clock_gettime(CLOCK_MONOTONIC)",
    "SDL_GetTicks()",
    "SDL_GetPerformanceCounter()",
    "GetTickCount()",
    "GetTickCount64()",
    "QueryPerformanceCounter()",
};

DeterministicTimer& DeterministicTimer::get() {
    static DeterministicTimer instance;
    return instance;
}

struct timespec DeterministicTimer::getTicks()
{
    return getTicks(SharedConfig::TIMETYPE_UNTRACKED_MONOTONIC);
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
        if (isTimeCallMonotonic(type))
            clock_gettime(CLOCK_MONOTONIC, &realtime);            
        else
            clock_gettime(CLOCK_REALTIME, &realtime);
        return realtime;
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return NonDeterministicTimer::get().getTicks(); // disable deterministic time
    }

    if ((type == SharedConfig::TIMETYPE_UNTRACKED_MONOTONIC) || GlobalState::isOwnCode()) {
        TimeHolder returnTicks = ticks + fakeExtraTicks;
        return returnTicks;
    }

    if ((type == SharedConfig::TIMETYPE_UNTRACKED_REALTIME)) {
        TimeHolder returnTicks = ticks + fakeExtraTicks;
        returnTicks += realtime_delta;
        return returnTicks;
    }

    LOGTRACE(LCF_TIMEGET);

    bool mainT = ThreadManager::isMainThread();

    /* Check for busy loops */
    if (mainT)
        BusyLoopDetection::increment(type);

    /* Update the count of time query calls, and advance time if reached the
     * limit. We use different counts for main and secondary threads because
     * advancing time by secondary threads increases the timer with nondeterministic values
     */

    int ticksExtra = 0;

    int gettimes_threshold = mainT ? Global::shared_config.main_gettimes_threshold[type]
                          : Global::shared_config.sec_gettimes_threshold[type];

    if (!insideFrameBoundary && /* Don't track if already inside a frame boundary */
        gettimes_threshold >= 0) {

        /* We actually track this time call */
        std::lock_guard<std::mutex> lock(ticks_mutex);
        int* gettimes_count = mainT ? &main_gettimes[type] : &sec_gettimes[type];
        (*gettimes_count)++;

        if(*gettimes_count > gettimes_threshold) {
            /*
             * We reached the limit of the number of calls.
             * We advance the deterministic timer by some value
             */
            int tickDelta = 1;

            LOG(LL_DEBUG, LCF_TIMESET, "WARNING! force-advancing time of type %d", type);

            ticksExtra += tickDelta;

            /* Reseting the number of calls from all functions */
            for (int i = 0; i < SharedConfig::TIMETYPE_NUMTRACKEDTYPES; i++) {
                main_gettimes[i] = 0;
                sec_gettimes[i] = 0;
            }
        }
    }
    else if (mainT && !insideFrameBoundary) {
        /* Still register calls to time functions, so that we can inform users
         * of potential options to tweak. */
        main_gettimes[type]++;
        if (main_gettimes[type] == ALERT_CALL_THRESHOLD) {            
            LOG(LL_WARN, LCF_TIMESET, "WARNING! many calls to function %s, you may need to enable time-tracking", gettimes_names[type]);
        }
    }

    if (ticksExtra) {
        /* Delay by ticksExtra ms. Arbitrary */
        struct timespec delay = {0, ticksExtra * 1000000};
        addDelay(delay);
    }

    TimeHolder returnTicks = ticks + fakeExtraTicks;
    if (!isTimeCallMonotonic(type))
        returnTicks += realtime_delta;
    return returnTicks;
}

void DeterministicTimer::addDelay(struct timespec delayTicks)
{
    LOG(LL_DEBUG, LCF_TIMESET | LCF_SLEEP, "%s call with delay %u.%010u sec", __func__, delayTicks.tv_sec, delayTicks.tv_nsec);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return NonDeterministicTimer::get().addDelay(delayTicks);

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

    if(!Global::shared_config.fastforward)
    {
        /* Sleep, because the caller would have yielded at least a little */
        NATIVECALL(sched_yield());
    }

    /* Don't trigger a non-draw frame when game is exiting */
    if (Global::is_exiting)
        return;

    /* We only allow the main thread to trigger a frame boundary! */
    bool mainT = ThreadManager::isMainThread();

    if (mainT && !insideFrameBoundary) {
        /* The threshold value to trigger a non-draw frame is critical for a 
         * number of games. Using a value too high will cause some games to
         * constantly trigger the time-tracking, or will get extra non-draw frames.
         
         * We want to document this with specific examples, so the next time we
         * modify this value, we will have test cases:
         * - In Hyper Light Drifter, setting a value too high (`thre = 2*base`) triggers
         *   a non-draw frame every 51 frames
         * - In Terraria, execution is sometimes normal, sometimes very slow with
         *   (`thre = 2*base`)
         */ 
        TimeHolder maxDeferredDelay = baseTimeIncrement;
        
        while(addedDelay > maxDeferredDelay) {
            /* We have built up too much delay. We must enter a frame boundary,
             * to advance the time.
             * This decrements addedDelay by (basically) how much it advances ticks
             */
            std::function<void()> dummy_draw;
            static RenderHUD dummy;
            frameBoundary(dummy_draw, dummy);
        }
    }
}

void DeterministicTimer::flushDelay()
{
    addedDelay = {0, 0};
}

void DeterministicTimer::exitFrameBoundary()
{
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return NonDeterministicTimer::get().exitFrameBoundary();

    LOGTRACE(LCF_TIMEGET);

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
    if (!(Global::shared_config.fastforward && (Global::shared_config.fastforward_mode & SharedConfig::FF_SLEEP))) {
        TimeHolder desiredTime = lastEnterTime + baseTimeIncrement * Global::shared_config.speed_divisor;

        /* Call the real nanosleep function */
        perfTimer.switchTimer(PerfTimer::IdleTimer);
#ifdef __unix__
        NATIVECALL(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &desiredTime, NULL));
#else
        /* nanosleep() uses relative time */
        TimeHolder sleepTime = desiredTime - currentTime;
        NATIVECALL(nanosleep(&sleepTime, NULL));
#endif
        perfTimer.switchTimer(PerfTimer::FrameTimer);
        
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

    insideFrameBoundary = false;
    frame_mutex.unlock();
}


TimeHolder DeterministicTimer::enterFrameBoundary()
{
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)
        return NonDeterministicTimer::get().enterFrameBoundary();

    frame_mutex.lock();
    LOGTRACE(LCF_TIMEGET);

    insideFrameBoundary = true;


    /*** First we update the state of the internal timer ***/

    /* We compute by how much we should advance the timer
     * to run exactly as the indicated framerate
     */
    TimeHolder timeIncrement = baseTimeIncrement;

    fractional_part += fractional_increment;
    while (fractional_part >= framerate_num)
    {
        timeIncrement.tv_nsec++;
        fractional_part -= framerate_num;
    }

    /* If we have less delay than the length of a frame, we advance ticks by
     * the remaining length. Otherwise, we don't increment ticks, and we
     * decrement addedDelay by the time increment.
     */
    if (timeIncrement > addedDelay) {
        TimeHolder deltaTicks = timeIncrement - addedDelay;
        ticks += deltaTicks;
        LOG(LL_DEBUG, LCF_TIMESET, "%s added %u.%010u", __func__, deltaTicks.tv_sec, deltaTicks.tv_nsec);
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
    while (fake_fractional_part >= framerate_num)
    {
        timeIncrement.tv_nsec++;
        fake_fractional_part -= framerate_num;
    }

    timeIncrement.tv_nsec+=1000000;

    if (timeIncrement > addedDelay) {
        fakeExtraTicks = timeIncrement - addedDelay;
    }
}

void DeterministicTimer::initialize(uint64_t initial_sec, uint64_t initial_nsec)
{
    ticks = {initial_sec, initial_nsec};
    
    realtime_delta = {Global::shared_config.initial_time_sec, Global::shared_config.initial_time_nsec};
    realtime_delta -= ticks;

    setFramerate(Global::shared_config.initial_framerate_num, Global::shared_config.initial_framerate_den);

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

void DeterministicTimer::setRealTime(uint32_t new_realtime_sec, uint32_t new_realtime_nsec)
{
    if (!new_realtime_sec && !new_realtime_nsec)
        return;

    TimeHolder th_real;
    th_real.tv_sec = new_realtime_sec;
    th_real.tv_nsec = new_realtime_nsec;
    realtime_delta = th_real - ticks;
}

void DeterministicTimer::setFramerate(uint32_t new_framerate_num, uint32_t new_framerate_den)
{
    if (!new_framerate_num)
        new_framerate_num = Global::shared_config.initial_framerate_num;

    if (!new_framerate_den)
        new_framerate_den = Global::shared_config.initial_framerate_den;
    
    if ((framerate_num != new_framerate_num) || (framerate_den != new_framerate_den)) {
        framerate_num = new_framerate_num;
        framerate_den = new_framerate_den;

        /* Update baseTimeIncrement */
        baseTimeIncrement.tv_sec = framerate_den / framerate_num;
        baseTimeIncrement.tv_nsec = 1000000000ULL * (uint64_t)(framerate_den % framerate_num) / framerate_num;

        /* Rreset fractional part */
        fractional_increment = 1000000000ULL * (uint64_t)(framerate_den % framerate_num) % framerate_num;
        fractional_part = 0;
    }
}

bool DeterministicTimer::isTimeCallMonotonic(SharedConfig::TimeCallType type)
{
    switch (type) {
        case SharedConfig::TIMETYPE_UNTRACKED_REALTIME:
        case SharedConfig::TIMETYPE_TIME:
        case SharedConfig::TIMETYPE_GETTIMEOFDAY:
        case SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME:
            return false;
        case SharedConfig::TIMETYPE_UNTRACKED_MONOTONIC:
        case SharedConfig::TIMETYPE_CLOCK:
        case SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC:
        case SharedConfig::TIMETYPE_SDLGETTICKS:
        case SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER:
            return true;
        case SharedConfig::TIMETYPE_GETTICKCOUNT:
        case SharedConfig::TIMETYPE_GETTICKCOUNT64:
        case SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER:
            return true; // TODO: I don't know!
        default:
            return true;
    }
}

SharedConfig::TimeCallType DeterministicTimer::clockToType(clockid_t clock)
{
    switch (clock) {
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_TAI:
            return SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME;
        default:
            return SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC;
    }
}

SharedConfig::TimeCallType DeterministicTimer::clockToTypeUntracked(clockid_t clock)
{
    switch (clock) {
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_TAI:
            return SharedConfig::TIMETYPE_UNTRACKED_REALTIME;
        default:
            return SharedConfig::TIMETYPE_UNTRACKED_MONOTONIC;
    }
}

}
