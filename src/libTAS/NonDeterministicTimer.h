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

#ifndef LIBTAS_NONDETERMINISTICTIMER_H_INCL
#define LIBTAS_NONDETERMINISTICTIMER_H_INCL

#include <time.h>
#include "TimeHolder.h"
#include "threadwrappers.h"

/* A simple timer that directly uses the system timer,
 * but is somewhat affected by fast-forward and frame advance.
 * It is not deterministic, but can be a good reference for
 * comparing against what the deterministic timer does.
 * (i.e. run the game first with this to see what the framerate is).
 *
 * If the game runs differently with this timer, then
 * probably this one is doing the more correct behavior,
 * especially if the chosen frame rate is wrong.
 * But many games will be unrecordable while using this timer.
 *
 * To enable this timer, set the framerate to 0.
 *
 * Code largely taken from Hourglass.
 */

class NonDeterministicTimer
{
public:

    /* Initialize the class members */
    void initialize(void);

    /* Update and return the time of the non deterministic timer */
    struct timespec getTicks(void);

    /* Function called when entering a frame boundary */
    void enterFrameBoundary(void);

    /* Function called when exiting a frame boundary,
     * to take into account the time spent in it
     */
    void exitFrameBoundary(void);

    /* Add a delay in the timer, and sleep */
    void addDelay(struct timespec delayTicks);

private:
    /* Current time of the timer */
    TimeHolder ticks;

    /* Last time value of the timer */
    TimeHolder lasttime;

    /* Are we inside a frame boundary? */
    bool inFB;

    /* The time of the last frame boundary enter */
    TimeHolder lastEnterTicks;

    /* The real time of the last frame boundary enter */
    TimeHolder lastEnterTime;

    /* The real time of the last frame boundary exit */
    TimeHolder lastExitTime;
};

extern NonDeterministicTimer nonDetTimer;

#endif
