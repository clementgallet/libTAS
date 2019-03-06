/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_DETERMINISTICTIMER_H_INCL
#define LIBTAS_DETERMINISTICTIMER_H_INCL

#include "TimeHolder.h"
#include "../shared/SharedConfig.h"
#include <mutex>

namespace libtas {
/* A timer that gives deterministic values, at least in the main thread.
 *
 * Deterministic means that calling frameBoundary() and querying this timer
 * in the same order will produce the same stream of results,
 * independently of anything else like the system clock or CPU speed.
 *
 * The main thread is defined as the thread that called SDL_Init(),
 *
 * The trick to making this timer deterministic and still run at a normal speed is:
 * we define a frame rate beforehand and always tell the game it is running that fast
 * from frame to frame. Then we do the waiting ourselves for each frame (using system timer).
 * this also lets us support "fast forward" without changing the values the game sees.
 */

class DeterministicTimer
{
    /* We will be using TimeHolder* <-> struct timespec* casting a lot, so we
     * must check if TimeHolder has a standard layout so that pointers to
     * TimeHolder objects can safely ba casted to struct timespec pointers. */
    static_assert(std::is_standard_layout<TimeHolder>::value, "TimeHolder must be standard layout");

public:

    /* Initialize the class members */
    void initialize(void);

    /* Update and return the time of the deterministic timer */
    struct timespec getTicks();
    struct timespec getTicks(SharedConfig::TimeCallType type);

    /* Returns true if the current frame boundary is acceptable for require_vsync mode */
    bool isFrameBoundary() { return insideFrameBoundary || sleepCalled; }

    /* Function called when entering a frame boundary */
    void enterFrameBoundary(void);

    /* Function called when exiting a frame boundary */
    void exitFrameBoundary(void);

    /* Add a delay in the timer, and sleep */
    void addDelay(struct timespec delayTicks, bool isSynthesized = false);

    /* Flush the accumulated timer delay. Used when game is exiting */
    void flushDelay();

    /* In specific situations, we must fake advancing timer.
     * This function temporarily fake adding ticks to the timer.
     * To be used like this:
     *     detTimer.fakeAdvanceTimer({0, 1000000});
     *     // Code that call GetTicks()
     *     detTimer.fakeAdvanceTimer({0, 0});
     */
    void fakeAdvanceTimer(struct timespec extraTicks);

private:

    /* By how much time do we increment the timer, excluding fractional part.
     * It only depends on the framerate setting.
     */
    TimeHolder baseTimeIncrement;

    /* Remainder when increasing the timer by 1/fps */
    unsigned int fractional_increment;

    /* Current sum of all fractional increments */
    unsigned int fractional_part;

    /* State of the deterministic timer */
    TimeHolder ticks;

    /*
     * Extra ticks to add to GetTicks().
     * Required for very specific situations.
     */
    TimeHolder fakeExtraTicks;

    /*
     * Real time of the last frame boundary enter.
     * Used for sleeping the right amount of time
     */
    TimeHolder lastEnterTime;

    /* Accumulated delay */
    TimeHolder addedDelay;

    /* Count for each time-getting method before time auto-advances to
     * avoid a freeze. Distinguish between main and secondary threads.
     */
    int main_gettimes[SharedConfig::TIMETYPE_NUMTRACKEDTYPES];
    int sec_gettimes[SharedConfig::TIMETYPE_NUMTRACKEDTYPES];

    /* Are we inside a frame boundary */
    bool insideFrameBoundary = false;

    /* Was sleep called at all since the last frame boundary */
    bool sleepCalled = false;

    /* Mutex to protect access to the ticks value */
    std::mutex mutex;
};

extern DeterministicTimer detTimer;

}

#endif
