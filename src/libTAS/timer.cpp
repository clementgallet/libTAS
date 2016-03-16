#include "timer.h"
#include "threads.h"
#include "frame.h"
#include "../shared/tasflags.h"

NonDeterministicTimer nonDetTimer;
DeterministicTimer detTimer;

void NonDeterministicTimer::initialize(void)
{
    ticks.tv_sec = 0;
    ticks.tv_nsec = 0;
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
}

void NonDeterministicTimer::exitFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&lastExitTime);
}

void NonDeterministicTimer::addDelay(struct timespec delayTicks)
{
    DEBUGLOGCALL(LCF_SLEEP | LCF_FRAME);

    if (tasflags.fastforward) {
        delayTicks.tv_sec = 0;
        delayTicks.tv_nsec = 0;
    }
    
    nanosleep_real(&delayTicks, NULL);
}

#define MAX_NONFRAME_GETTIMES 4000

struct timespec DeterministicTimer::getTicks(TimeCallType type=TIMETYPE_UNTRACKED)
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FREQUENT);

    if(tasflags.framerate == 0) {
        struct timespec rv = nonDetTimer.getTicks();
        return rv; // 0 framerate means disable deterministic timer
    }

    bool isFrameThread = isMainThread();

    /* Only the main thread can modify the timer */
    if(!isFrameThread)
    {
        if(type != TIMETYPE_UNTRACKED)
        {

            /* Well, actually, if another thread get the time too many time,
             * we temporarily consider it as the main thread.
             * This can lead to desyncs
             */
            if(getTimes >= MAX_NONFRAME_GETTIMES)
            {
                if(getTimes == MAX_NONFRAME_GETTIMES)
                    debuglog(LCF_TIMEGET | LCF_DESYNC, "Temporarily assuming main thread");
                isFrameThread = true;
            }
            getTimes++;
        }
    }

    if (isFrameThread)
    {
        /* Only do this in the main thread so as to not dirty the timer with nondeterministic values
         * (not to mention it would be extremely multithreading-unsafe without using sync primitives otherwise)
         */

        int ticksExtra = 0;

        if (type != TIMETYPE_UNTRACKED)
        {
            debuglog(LCF_TIMESET | LCF_FREQUENT, "subticks ", type, " increased");
            altGetTimes[type]++;

            if(altGetTimes[type] > altGetTimeLimits[type]) {
                /* 
                 * We reached the limit of the number of calls.
                 * We advance the deterministic timer by some value
                 */
                int tickDelta = 1;

                debuglog(LCF_TIMESET | LCF_FREQUENT, "WARNING! force-advancing time of type ", type);

                ticksExtra += tickDelta;

                /* Reseting the number of calls from all functions */
                for (int i = 0; i < TIMETYPE_NUMTRACKEDTYPES; i++)
                    altGetTimes[i] = 0;

            }
        }

#if 0 // Not sure we need this
        if (getTimes == MAX_NONFRAME_GETTIMES && !frameThreadId)
            debuglog(LCF_TIMESET, "WARNING! temporarily switched to non-deterministic timer (%d)\n", ticks);
        getTimes++;
#endif
        if(ticksExtra) {
            /* Delay by ticksExtra ms. Arbitrary */
            struct timespec delay = {0, ticksExtra * 1000000};
            addDelay(delay);
        }
    }

    return *(struct timespec*)&ticks;
}


void DeterministicTimer::addDelay(struct timespec delayTicks)
{
    debuglog(LCF_TIMESET | LCF_SLEEP, __func__, " call with delay ", delayTicks.tv_sec * 1000000000 + delayTicks.tv_nsec, " nsec");

    if(tasflags.framerate == 0) // 0 framerate means disable deterministic timer
        return nonDetTimer.addDelay(delayTicks);

    /* Deferring as much of the delay as possible
     * until the place where the regular per-frame delay is applied
     * gives the smoothest results.
     * However, there must be a limit,
     * otherwise it could easily build up and make us freeze (in some games)
     */

    TimeHolder maxDeferredDelay = timeIncrement * 6;

    addedDelay += *(TimeHolder*)&delayTicks;
    ticks += *(TimeHolder*)&delayTicks;
    forceAdvancedTicks += *(TimeHolder*)&delayTicks;

    if(!tasflags.fastforward)
    {
        /* Sleep, because the caller would have yielded at least a little */
        struct timespec nosleep = {0, 0};
        nanosleep_real(&nosleep, NULL);
    }

    while(addedDelay > maxDeferredDelay)
    {
        drawFB = false;

        /* This decrements addedDelay by (basically) how much it advances ticks */
        frameBoundary();

    }
}

void DeterministicTimer::exitFrameBoundary()
{
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);

    /* Reset the counts of each time get function */
    for(int i = 0; i < TIMETYPE_NUMTRACKEDTYPES; i++)
        altGetTimes[i] = 0;

    if(tasflags.framerate == 0)
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
    DEBUGLOGCALL(LCF_TIMEGET | LCF_FRAME);

    if(tasflags.framerate == 0)
        return nonDetTimer.enterFrameBoundary(); // 0 framerate means disable deterministic timer

    unsigned int integer_increment = 1000000000 / tasflags.framerate;
    unsigned int fractional_increment = 1000000000 % tasflags.framerate;

    timeIncrement.tv_sec = 0;
    timeIncrement.tv_nsec = integer_increment;

    fractional_part += fractional_increment;
    if (fractional_part >= tasflags.framerate)
    {
        timeIncrement.tv_nsec++;
        fractional_part -= tasflags.framerate;
    }

    /* Subtract out ticks that were made when calling GetTicks() */
    //TimeHolder takenTicks = (ticks - lastEnterTicks) + forceAdvancedTicks;
    TimeHolder takenTicks = (ticks - lastEnterTicks) /*+ forceAdvancedTicks*/;

    //int getCurrentFramestampLogical();

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

    /* Get the current actual time */
    TimeHolder currentTime;
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&currentTime);

    // calculate the target time we wanted to be at now
    //DWORD timeScale = tasflags.timescale;
    //DWORD timeScaleDiv = tasflags.timescaleDivisor;
    //int desiredDeltaTime = ((int)(newTicks - lastOvershot)) * (int)timeScaleDiv;
    //if(timeScale > 1)
    //    desiredDeltaTime /= (int)timeScale;
    //
    TimeHolder desiredTime = lastEnterTime + timeIncrement;

    TimeHolder deltaTime = desiredTime - currentTime;

    //AdvanceTimeAndMixAll(newTicks);

    /* If we are not fast forwarding, and not the first frame,
     * then we wait the delta amount of time
     */
    if (!tasflags.fastforward && lastEnterValid) {

        /* Check that we wait for a positive time */
        if ((deltaTime.tv_sec > 0) || ((deltaTime.tv_sec == 0) && (deltaTime.tv_nsec >= 0))) {
            //debuglog(LCF_SLEEP, "Add a wait of ", deltaTime.tv_sec * 1000000000 + deltaTime.tv_nsec, " nsec");
            nanosleep_real((struct timespec*)&deltaTime, NULL);
        }
    }

    /* 
     * WARNING: This time update is not done in Hourglass,
     * maybe intentionally.
     */
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&currentTime);

    lastEnterTime = currentTime;

    lastEnterTicks = ticks;
    lastEnterValid = true;
}

void DeterministicTimer::initialize(void)
{
    getTimes = 0;
    ticks = {0, 0};
    fractional_part = tasflags.framerate / 2;
    clock_gettime_real(CLOCK_MONOTONIC, (struct timespec*)&lastEnterTime);
    lastEnterTicks = ticks;

    for(int i = 0; i < TIMETYPE_NUMTRACKEDTYPES; i++)
        altGetTimes[i] = 0;
    for(int i = 0; i < TIMETYPE_NUMTRACKEDTYPES; i++)
        altGetTimeLimits[i] = 100;

    forceAdvancedTicks = {0, 0};
    addedDelay = {0, 0};
    lastEnterValid = false;

    drawFB = true;
}
