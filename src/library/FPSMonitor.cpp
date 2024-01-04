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

#include "FPSMonitor.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"
#include "DeterministicTimer.h"
#include "general/timewrappers.h" // clock_gettime
#include "TimeHolder.h"

#include <array>
#include <stdint.h>

namespace libtas {

/* Compute real and logical fps */
void FPSMonitor::tickFrame(uint64_t framecount, float* fps, float* lfps)
{
    /* Do we have enough values to compute fps? */
    static bool can_output = false;

    /* Frequency of FPS computing (every n frames) */
    static int fps_refresh_freq = 15;

    /* Computations include values from past n calls */
    static const int history_length = 10;

    static std::array<uint64_t, history_length> lastFrames;
    static std::array<TimeHolder, history_length> lastTimes;
    static std::array<TimeHolder, history_length> lastTicks;

    static int refresh_counter = 0;
    static int compute_counter = 0;

    /* Immedialty reset fps computing frequency if not fast-forwarding */
    if (!Global::shared_config.fastforward) {
        fps_refresh_freq = 10;
    }

    if (++refresh_counter >= fps_refresh_freq) {
        refresh_counter = 0;

        /* Update frame */
        uint64_t lastFrame = lastFrames[compute_counter];
        lastFrames[compute_counter] = framecount;

        /* Update current time */
        TimeHolder lastTime = lastTimes[compute_counter];
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastTimes[compute_counter]));

        /* Update current ticks */
        TimeHolder lastTick = lastTicks[compute_counter];
        lastTicks[compute_counter] = DeterministicTimer::get().getTicks();

        uint64_t deltaFrames = framecount - lastFrame;

        /* Compute real fps (number of ticks per second) */
        TimeHolder deltaTime = lastTimes[compute_counter] - lastTime;

        /* Compute logical fps (number of ticks per timer second) */
        TimeHolder deltaTicks = lastTicks[compute_counter] - lastTick;

        if (++compute_counter >= history_length) {
            compute_counter = 0;
            can_output = true;
        }

        if (can_output) {
            *fps = static_cast<float>(deltaFrames) * 1000000000.0f / (deltaTime.tv_sec * 1000000000.0f + deltaTime.tv_nsec);
            *lfps = static_cast<float>(deltaFrames) * 1000000000.0f / (deltaTicks.tv_sec * 1000000000.0f + deltaTicks.tv_nsec);

            /* Update fps computing frequency if fast-forwarding */
            if (Global::shared_config.fastforward) {
                fps_refresh_freq = static_cast<int>(*fps) / 4;
            }
        }
    }
}

float FPSMonitor::tickRedraw()
{
    /* Computations include values from past n calls */
    static const int history_length = 10;

    static std::array<TimeHolder, history_length> lastTimes{};

    static int compute_counter = 0;

    /* Update current time */
    const TimeHolder lastTime = lastTimes[compute_counter];
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastTimes[compute_counter]));
    const TimeHolder currentTime = lastTimes[compute_counter];

    if (++compute_counter >= history_length) {
        compute_counter = 0;
    }

    if (lastTime.tv_sec == 0) {
        /* We didn't fill yet the whole array */
        return 0.0f;
    }
        
    /* Compute real fps (number of ticks per second) */
    TimeHolder deltaTime = currentTime - lastTime;

    return static_cast<float>(history_length) * 1000000000.0f / (deltaTime.tv_sec * 1000000000.0f + deltaTime.tv_nsec);
}


}
