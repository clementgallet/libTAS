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

#include "frame.h"
#include "../shared/tasflags.h"
#include "../shared/AllInputs.h"
#include "../shared/messages.h"
#include "inputs/inputs.h" // AllInputs ai object
#include "inputs/sdlinputevents.h"
#include "socket.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "avdumping.h"
#include "EventQueue.h"
#include "sdlwindows.h"
#include <mutex>
#include <iomanip>

/* Compute real and logical fps */
static bool computeFPS(bool drawFB, float& fps, float& lfps)
{
    static int computeCounter = 0;
    static TimeHolder lastTime;
    static TimeHolder lastTicks;
    static int drawFrameCount = 0;
    static int lastFrameCount = 0;

    if (drawFB)
        drawFrameCount++;

    if (++computeCounter > 60) {
        computeCounter = 0;

        TimeHolder currentTime;
        orig::clock_gettime(CLOCK_MONOTONIC, &currentTime);

        struct timespec tsTicks = detTimer.getTicks(TIMETYPE_UNTRACKED);
        TimeHolder currentTicks;
        currentTicks = tsTicks;

        /* Compute real fps (number of drawn screens per second) */
        TimeHolder deltaTime = currentTime - lastTime;
        fps = static_cast<float>(drawFrameCount - lastFrameCount) * 1000000000.0f / (deltaTime.tv_sec * 1000000000.0f + deltaTime.tv_nsec);

        /* Compute logical fps (number of drawn screens per timer second) */
        TimeHolder deltaTicks = currentTicks - lastTicks;
        lfps = static_cast<float>(drawFrameCount - lastFrameCount) * 1000000000.0f / (deltaTicks.tv_sec * 1000000000.0f + deltaTicks.tv_nsec);

        lastTime = currentTime;
        lastTicks = currentTicks;
        lastFrameCount = drawFrameCount;

        return true;
    }

    return false;
}

/* Deciding if we actually draw the frame */
static bool skipDraw(void)
{
    static int skipCounter = 0;
    if (tasflags.fastforward) {
        if (skipCounter++ > 10)
            skipCounter = 0;
    }
    else
        skipCounter = 0;

    return skipCounter;
}

std::mutex frameMutex;

#ifdef LIBTAS_ENABLE_HUD
void frameBoundary(bool drawFB, std::function<void()> draw, RenderHUD& hud)
#else
void frameBoundary(bool drawFB, std::function<void()> draw)
#endif
{
    std::lock_guard<std::mutex> guard(frameMutex);
    debuglog(LCF_TIMEFUNC | LCF_FRAME, "Enter frame boundary");

    detTimer.enterFrameBoundary();

    /* Audio mixing is done above, so encode must be called after */
#ifdef LIBTAS_ENABLE_AVDUMPING
    /* Dumping audio and video */
    if (tasflags.av_dumping) {
        /* Write the current frame */
        int enc = encodeOneFrame(frame_counter);
        if (enc != 0) {
            /* Encode failed, disable AV dumping */
            closeAVDumping();
            tasflags.av_dumping = 0;
        }
    }
#endif

#ifdef LIBTAS_ENABLE_HUD
    hud.renderFrame(frame_counter);
    hud.renderInputs(ai);
#endif

    threadState.setNative(true);
    if (!skipDraw())
        draw();
    threadState.setNative(false);

    sendMessage(MSGB_START_FRAMEBOUNDARY);
    sendData(&frame_counter, sizeof(unsigned long));

    proceed_commands();

    /* Push native SDL events into our emulated event queue */
    pushNativeEvents();

    /* Push generated events.
     * This must be done after getting the new inputs. */
    generateSDLKeyUpEvents();
    generateSDLKeyDownEvents();
    if (frame_counter == 0)
        generateSDLControllerAdded();
    generateSDLControllerEvents();
    generateSDLMouseMotionEvents();
    generateSDLMouseButtonEvents();

    ++frame_counter;

    /* Print FPS */
    float fps, lfps;
    if (computeFPS(drawFB, fps, lfps)) {
        debuglog(LCF_TIMEFUNC | LCF_FRAME, "fps: ", std::fixed, std::setprecision(1), fps, " lfps: ", lfps);
        updateTitle(fps, lfps);
    }

    detTimer.exitFrameBoundary();
    debuglog(LCF_TIMEFUNC | LCF_FRAME, "Leave frame boundary");
}

void proceed_commands(void)
{
    int message;
    while (1)
    {
        receiveData(&message, sizeof(int));

        switch (message)
        {
            case MSGN_TASFLAGS:
                receiveData(&tasflags, sizeof(struct TasFlags));
                break;

            case MSGN_END_FRAMEBOUNDARY:
                return;

            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(struct AllInputs));
                break;

        }
    }
}

