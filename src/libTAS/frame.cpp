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
#include "../shared/AllInputs.h"
#include "../shared/messages.h"
#include "../shared/SharedConfig.h"
#include "inputs/inputs.h" // AllInputs ai object
#include "inputs/sdlinputevents.h"
#include "../shared/sockethelpers.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "AVEncoder.h"
#include "sdlwindows.h"
#include "sdlevents.h"
#include <iomanip>
#include "timewrappers.h" // clock_gettime
#include "threadwrappers.h" // isMainThread()
#include "checkpoint/ThreadManager.h"
#include "screenpixels.h"

namespace libtas {

#define SAVESTATE_FILESIZE 1024
static char savestatepath[SAVESTATE_FILESIZE];

static void proceed_commands(void);

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
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &currentTime));

        struct timespec tsTicks = detTimer.getTicks();
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
    if (shared_config.fastforward) {
        if (skipCounter++ > 10) // TODO: Tweak this value based on fps
            skipCounter = 0;
    }
    else
        skipCounter = 0;

    return skipCounter;
}

#ifdef LIBTAS_ENABLE_HUD
void frameBoundary(bool drawFB, std::function<void()> draw, RenderHUD& hud)
#else
void frameBoundary(bool drawFB, std::function<void()> draw)
#endif
{
    debuglog(LCF_FRAME, "Enter frame boundary");

    if (!isMainThread())
        debuglog(LCF_ERROR | LCF_FRAME, "Warning! Entering a frame boudary from a secondary thread!");

    detTimer.enterFrameBoundary();

#ifdef LIBTAS_ENABLE_HUD
    if (shared_config.hud_encode) {
        if (shared_config.hud_framecount)
            hud.renderFrame(frame_counter);
        if (shared_config.hud_inputs)
            hud.renderInputs(ai);
    }
#endif

    /* Audio mixing is done above, so encode must be called after */
#ifdef LIBTAS_ENABLE_AVDUMPING
    /* Dumping audio and video */
    if (shared_config.av_dumping) {

        /* First, create the AVEncoder is needed */
        if (!avencoder) {
            debuglog(LCF_DUMP, "Start AV dumping on file ", av_filename);
            avencoder.reset(new AVEncoder(gameWindow, video_opengl, av_filename, frame_counter));
        }

        /* Write the current frame */
        int enc = avencoder->encodeOneFrame(frame_counter, drawFB);
        if (enc < 0) {
            /* Encode failed, disable AV dumping */
            avencoder.reset(nullptr);
            debuglog(LCF_ALERT, "Encoding to ", av_filename, " failed because:\n", avencoder->getErrorMsg());
            shared_config.av_dumping = false;
            sendMessage(MSGB_ENCODE_FAILED);
        }
    }
    else {
        /* If there is still an encoder object, it means we just stopped
         * encoding, so we must delete the encoder object.
         */
        if (avencoder) {
            debuglog(LCF_DUMP, "Stop AV dumping");
            avencoder.reset(nullptr);
        }
    }
#endif

#ifdef LIBTAS_ENABLE_HUD
    if (!shared_config.hud_encode) {
        if (shared_config.hud_framecount)
            hud.renderFrame(frame_counter);
        if (shared_config.hud_inputs)
            hud.renderInputs(ai);
    }
#endif

    if (!skipDraw()) {
        if (drawFB && shared_config.save_screenpixels) {
            /* We must save the screen pixels just before drawing, in case we
             * load a savestate here, so we can redraw the screen.
             */
            getScreenPixels(nullptr, nullptr);
        }

        draw();
    }

    /* Send error messages */
    std::string alert;
    while (getAlertMsg(alert)) {
        sendMessage(MSGB_ALERT_MSG);
        sendString(alert);
    }

    sendMessage(MSGB_FRAMECOUNT);
    sendData(&frame_counter, sizeof(unsigned long));

    sendMessage(MSGB_START_FRAMEBOUNDARY);

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
    static float fps, lfps = 0;
    if (computeFPS(drawFB, fps, lfps)) {
        debuglog(LCF_FRAME, "fps: ", std::fixed, std::setprecision(1), fps, " lfps: ", lfps);
    }
    updateTitle(fps, lfps);

    detTimer.exitFrameBoundary();
    debuglog(LCF_FRAME, "Leave frame boundary");
}

static void proceed_commands(void)
{
    int message;
    while (1)
    {
        receiveData(&message, sizeof(int));

        switch (message)
        {
            case MSGN_USERQUIT:
                pushQuitEvent();
                break;

            case MSGN_CONFIG:
                receiveData(&shared_config, sizeof(SharedConfig));
                break;

            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET | LCF_FRAME, "Receiving dump filename");
                /* FIXME: Memory leak! */
                size_t dump_len;
                receiveData(&dump_len, sizeof(size_t));
                /* TODO: Put all this in TasFlags class methods */
                av_filename = static_cast<char*>(malloc(dump_len+1));
                receiveData(av_filename, dump_len);
                av_filename[dump_len] = '\0';
                debuglog(LCF_SOCKET | LCF_FRAME, "File ", av_filename);
                break;

            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(AllInputs));
                break;

            case MSGN_SAVESTATE:
                /* Get the savestate path */
                receiveCString(savestatepath);

                ThreadManager::checkpoint(savestatepath);
                /* Don't forget that when we load a savestate, the game continues
                 * from here and not from ThreadManager::restore() under.
                 * To check if we did restored or returned from a checkpoint,
                 * we look at variable ThreadManager::restoreInProgress.
                 */

                if (ThreadManager::restoreInProgress) {
                    if (shared_config.save_screenpixels) {
                        /* If we restored from a savestate, refresh the screen */
                        setScreenPixels(gameWindow);
                    }

                    /* We must send again the frame count because it probably has
                     * changed.
                     */
                    sendMessage(MSGB_FRAMECOUNT);
                    sendData(&frame_counter, sizeof(unsigned long));
                }

                break;

            case MSGN_LOADSTATE:
                /* Get the savestate path */
                receiveCString(savestatepath);

                ThreadManager::restore(savestatepath);

                /* If restoring failed, we return here. We must still send the
                 * frame count because the program does not know that restore
                 * failed.
                 */
                sendMessage(MSGB_FRAMECOUNT);
                sendData(&frame_counter, sizeof(unsigned long));

                break;

            case MSGN_END_FRAMEBOUNDARY:
                return;

            default:
                debuglog(LCF_ERROR | LCF_SOCKET | LCF_FRAME, "Unknown message received");
                return;
        }
    }
}

}
