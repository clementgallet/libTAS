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
#include "global.h" // shared_config
#include "inputs/inputs.h" // AllInputs ai object
#include "inputs/inputevents.h"
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
#include "ScreenCapture.h"
#include "WindowTitle.h"

namespace libtas {

#define SAVESTATE_FILESIZE 1024
static char savestatepath[SAVESTATE_FILESIZE];

/* Store the number of nondraw frames */
static unsigned int nondraw_frame_counter = 0;

#ifdef LIBTAS_ENABLE_HUD
static void receive_messages(std::function<void()> draw, RenderHUD& hud);
#else
static void receive_messages(std::function<void()> draw);
#endif

/* Compute real and logical fps */
static void computeFPS(float& fps, float& lfps)
{
    static const int frame_segment_size = 60;

    static std::array<TimeHolder, frame_segment_size> lastTimes;
    static std::array<TimeHolder, frame_segment_size> lastTickss;

    static int computeCounter = 0;

    /* Update current time */
    TimeHolder lastTime = lastTimes[computeCounter%60];
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastTimes[computeCounter%60]));

    /* Update current ticks */
    TimeHolder lastTicks = lastTickss[computeCounter%60];
    struct timespec tsTicks = detTimer.getTicks();
    lastTickss[computeCounter%60] = tsTicks;


    /* Compute real fps (number of drawn screens per second) */
    TimeHolder deltaTime = lastTimes[computeCounter%60] - lastTime;

    /* Compute logical fps (number of drawn screens per timer second) */
    TimeHolder deltaTicks = lastTickss[computeCounter%60] - lastTicks;

    computeCounter++;

    if (computeCounter >= 60) {
        fps = static_cast<float>(frame_segment_size) * 1000000000.0f / (deltaTime.tv_sec * 1000000000.0f + deltaTime.tv_nsec);
        lfps = static_cast<float>(frame_segment_size) * 1000000000.0f / (deltaTicks.tv_sec * 1000000000.0f + deltaTicks.tv_nsec);
    }

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

    if (!drawFB)
        nondraw_frame_counter++;

    if (!isMainThread())
        debuglog(LCF_ERROR | LCF_FRAME, "Warning! Entering a frame boudary from a secondary thread!");

    detTimer.enterFrameBoundary();

    /* Decide if we skip drawing to the screen because of fastforward. We must
     * call it once per frame boundary, because it raises an internal counter.
     */
    bool skipping_draw = skipDraw();

    /* We must save the screen pixels before drawing for the following cases:
     * - we set screen redraw when loading savestates
     * - we show the inputs in the HUD, so that we can show preview inputs
     * Also, we must take care of saving before rendering the HUD.
     *
     * TODO: What should we do for nondraw frames ???
     */
    if (!skipping_draw) {
        if (drawFB && (shared_config.save_screenpixels || shared_config.hud_inputs)) {
            ScreenCapture::getPixels(nullptr, nullptr);
        }
    }

#ifdef LIBTAS_ENABLE_HUD
    if (shared_config.hud_encode) {
        if (shared_config.hud_framecount) {
            hud.renderFrame(frame_counter);
            hud.renderNonDrawFrame(nondraw_frame_counter);
        }
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
            debuglog(LCF_DUMP, "Start AV dumping on file ", AVEncoder::dumpfile);
            avencoder.reset(new AVEncoder(gameWindow, frame_counter));
        }

        /* Write the current frame */
        int enc = avencoder->encodeOneFrame(frame_counter, drawFB);
        if (enc < 0) {
            /* Encode failed, disable AV dumping */
            avencoder.reset(nullptr);
            debuglog(LCF_ALERT, "Encoding to ", AVEncoder::dumpfile, " failed because:\n", avencoder->getErrorMsg());
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
        if (shared_config.hud_framecount) {
            hud.renderFrame(frame_counter);
            hud.renderNonDrawFrame(nondraw_frame_counter);
        }
        if (shared_config.hud_inputs)
            hud.renderInputs(ai);
    }
#endif

    if (!skipping_draw) {
        NATIVECALL(draw());
    }

    /* Send error messages */
    std::string alert;
    while (getAlertMsg(alert)) {
        sendMessage(MSGB_ALERT_MSG);
        sendString(alert);
    }

    /* Send framecount and internal time */
    sendMessage(MSGB_FRAMECOUNT_TIME);
    sendData(&frame_counter, sizeof(unsigned long));
    struct timespec ticks = detTimer.getTicks();
    sendData(&ticks, sizeof(struct timespec));

    /* Send GameInfo struct if needed */
    if (game_info.tosend) {
        sendMessage(MSGB_GAMEINFO);
        sendData(&game_info, sizeof(game_info));
        game_info.tosend = false;
    }

    /* Last message to send */
    sendMessage(MSGB_START_FRAMEBOUNDARY);

#ifdef LIBTAS_ENABLE_HUD
    receive_messages(draw, hud);
#else
    receive_messages(draw);
#endif

    if ((game_info.video & GameInfo::SDL1) || (game_info.video & GameInfo::SDL2)) {
        /* Push native SDL events into our emulated event queue */
        pushNativeEvents();
    }

    /* Push generated events.
     * This must be done after getting the new inputs. */
    generateKeyUpEvents();
    generateKeyDownEvents();
    if (frame_counter == 0)
        generateControllerAdded();
    generateControllerEvents();
    generateMouseMotionEvents();
    generateMouseButtonEvents();

    ++frame_counter;

    /* Print FPS */
    static float fps, lfps = 0;
    if (drawFB) {
        computeFPS(fps, lfps);
        debuglog(LCF_FRAME, "fps: ", std::fixed, std::setprecision(1), fps, " lfps: ", lfps);
    }
    WindowTitle::update(fps, lfps);

    detTimer.exitFrameBoundary();
    debuglog(LCF_FRAME, "Leave frame boundary");
}

#ifdef LIBTAS_ENABLE_HUD
static void receive_messages(std::function<void()> draw, RenderHUD& hud)
#else
static void receive_messages(std::function<void()> draw)
#endif
{
    while (1)
    {
        int message = receiveMessage();
        AllInputs preview_ai;

        switch (message)
        {
            case MSGN_USERQUIT:
                pushQuitEvent();
                break;

            case MSGN_CONFIG:
                receiveData(&shared_config, sizeof(SharedConfig));
                break;

#ifdef LIBTAS_ENABLE_AVDUMPING
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET | LCF_FRAME, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglog(LCF_SOCKET | LCF_FRAME, "File ", AVEncoder::dumpfile);
                break;
#endif
            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(AllInputs));
                break;

            case MSGN_PREVIEW_INPUTS:
                receiveData(&preview_ai, sizeof(AllInputs));

#ifdef LIBTAS_ENABLE_HUD
                if (shared_config.hud_inputs) {
                    ScreenCapture::setPixels();

                    if (shared_config.hud_framecount) {
                        hud.renderFrame(frame_counter);
                        hud.renderNonDrawFrame(nondraw_frame_counter);
                    }
                    hud.renderInputs(ai);
                    hud.renderPreviewInputs(preview_ai);

                    NATIVECALL(draw());
                }
#endif

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
                    /* Tell the program that the loading succeeded */
                    sendMessage(MSGB_LOADING_SUCCEEDED);

                    /* After loading, the game and the program no longer store
                     * the same information, so they must communicate to be
                     * synced again.
                     */

                    /* We receive the shared config struct */
                    message = receiveMessage();
                    MYASSERT(message == MSGN_CONFIG)
                    receiveData(&shared_config, sizeof(SharedConfig));

                    /* We must send again the frame count and time because it
                     * probably has changed.
                     */
                    sendMessage(MSGB_FRAMECOUNT_TIME);
                    sendData(&frame_counter, sizeof(unsigned long));
                    struct timespec ticks = detTimer.getTicks();
                    sendData(&ticks, sizeof(struct timespec));

                    if (shared_config.save_screenpixels) {
                        /* If we restored from a savestate, refresh the screen */

                        ScreenCapture::setPixels();

#ifdef LIBTAS_ENABLE_HUD
                        if (shared_config.hud_framecount) {
                            hud.renderFrame(frame_counter);
                            hud.renderNonDrawFrame(nondraw_frame_counter);
                        }
                        if (shared_config.hud_inputs)
                            hud.renderInputs(ai);
#endif

                        NATIVECALL(draw());
                    }

                }

                break;

            case MSGN_LOADSTATE:
                /* Get the savestate path */
                receiveCString(savestatepath);

                ThreadManager::restore(savestatepath);

                /* If restoring failed, we return here. We still send the
                 * frame count and time because the program will pull a
                 * message in either case.
                 */
                sendMessage(MSGB_FRAMECOUNT_TIME);
                sendData(&frame_counter, sizeof(unsigned long));
                {
                    struct timespec ticks = detTimer.getTicks();
                    sendData(&ticks, sizeof(struct timespec));
                }

                break;

            case MSGN_STOP_ENCODE:
#ifdef LIBTAS_ENABLE_AVDUMPING
                if (avencoder) {
                    debuglog(LCF_DUMP, "Stop AV dumping");
                    avencoder.reset(nullptr);
                    shared_config.av_dumping = false;

                    /* Update title without changing fps */
                    WindowTitle::update(-1, -1);
                }
#endif
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
