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

#include "frame.h"
#include "../shared/AllInputs.h"
#include "../shared/messages.h"
#include "global.h" // shared_config
#include "inputs/inputs.h" // AllInputs ai object
#include "inputs/inputevents.h"
#include "../shared/sockethelpers.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "encoding/AVEncoder.h"
#include "sdlwindows.h"
#include "sdlevents.h"
#include <iomanip>
#include "timewrappers.h" // clock_gettime
#include "threadwrappers.h" // isMainThread()
#include "checkpoint/ThreadManager.h"
#include "checkpoint/Checkpoint.h"
#include "ScreenCapture.h"
#include "WindowTitle.h"
#include "EventQueue.h"

namespace libtas {

/* Frame counter */
unsigned long framecount = 0;

/* Store the number of nondraw frames */
static unsigned int nondraw_framecount = 0;

#ifdef LIBTAS_ENABLE_HUD
static void receive_messages(std::function<void()> draw, RenderHUD& hud);
#else
static void receive_messages(std::function<void()> draw);
#endif

/* Compute real and logical fps */
static void computeFPS(float& fps, float& lfps)
{
    /* Do We have enough values to compute fps? */
    static bool can_output = false;

    /* Frequency of FPS computing (every n frames) */
    static int fps_refresh_freq = 15;

    /* Computations include values from past n calls */
    static const int history_length = 10;

    static std::array<unsigned long, history_length> lastFrames;
    static std::array<TimeHolder, history_length> lastTimes;
    static std::array<TimeHolder, history_length> lastTicks;

    static int refresh_counter = 0;
    static int compute_counter = 0;

    /* Immedialty reset fps computing frequency if not fast-forwarding */
    if (!shared_config.fastforward) {
        fps_refresh_freq = 10;
    }

    if (++refresh_counter >= fps_refresh_freq) {
        refresh_counter = 0;

        /* Update frame */
        unsigned long lastFrame = lastFrames[compute_counter];
        lastFrames[compute_counter] = framecount;

        /* Update current time */
        TimeHolder lastTime = lastTimes[compute_counter];
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastTimes[compute_counter]));

        /* Update current ticks */
        TimeHolder lastTick = lastTicks[compute_counter];
        lastTicks[compute_counter] = detTimer.getTicks();

        unsigned long deltaFrames = framecount - lastFrame;

        /* Compute real fps (number of drawn screens per second) */
        TimeHolder deltaTime = lastTimes[compute_counter] - lastTime;

        /* Compute logical fps (number of drawn screens per timer second) */
        TimeHolder deltaTicks = lastTicks[compute_counter] - lastTick;

        if (++compute_counter >= history_length) {
            compute_counter = 0;
            can_output = true;
        }

        if (can_output) {
            fps = static_cast<float>(deltaFrames) * 1000000000.0f / (deltaTime.tv_sec * 1000000000.0f + deltaTime.tv_nsec);
            lfps = static_cast<float>(deltaFrames) * 1000000000.0f / (deltaTicks.tv_sec * 1000000000.0f + deltaTicks.tv_nsec);

            /* Update fps computing frequency if fast-forwarding */
            if (shared_config.fastforward) {
                fps_refresh_freq = static_cast<int>(fps) / 4;
            }
        }
    }
}

/* Deciding if we actually draw the frame */
static bool skipDraw(float fps)
{
    static unsigned int skip_counter = 0;

    /* Don't skip if not fastforwarding */
    if (!shared_config.fastforward)
        return false;

    /* Don't skip if frame-advancing */
    if (!shared_config.running)
        return false;

    /* Never skip a draw when encoding. */
    if (shared_config.av_dumping)
        return false;

    /* Always skip if rendering skip mode */
    if (shared_config.fastforward_mode & SharedConfig::FF_RENDERING)
        return true;

    unsigned int skip_freq = 1;

    /* I want to display about 8 effective frames per second, so I divide
     * the fps value by 8 to get the skip frequency.
     * Moreover, it is better to have bands of the same skip frequency, so
     * I take the next highest power of 2.
     * Because the value is already in a float, I can use this neat trick from
     * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
     */

    if (fps > 1) {
        fps--;
        memcpy(&skip_freq, &fps, sizeof(int));
        skip_freq = 1U << ((skip_freq >> 23) - 126 - 3); // -3 -> divide by 8
    }

    /* At least skip 3 frames out of 4 */
    if (skip_freq < 4)
        skip_freq = 4;

    if (++skip_counter >= skip_freq) {
        skip_counter = 0;
        return false;
    }

    return true;
}

#ifdef LIBTAS_ENABLE_HUD
void frameBoundary(bool drawFB, std::function<void()> draw, RenderHUD& hud, bool restore_screen)
#else
void frameBoundary(bool drawFB, std::function<void()> draw, bool restore_screen)
#endif
{
    static float fps, lfps = 0;

    debuglog(LCF_FRAME, "Enter frame boundary");
    ThreadManager::setMainThread();

    /* If the game is exiting, dont process the frame boundary, just draw and exit */
    if (is_exiting) {
        debuglog(LCF_FRAME, "Game is exiting: skipping frame boundary");
        detTimer.flushDelay();
        NATIVECALL(draw());
        return;
    }

    /*** Update time ***/

    /* First, increase the frame count */
    ++framecount;

    /* Compute new FPS values */
    if (drawFB) {
        computeFPS(fps, lfps);
        debuglog(LCF_FRAME, "fps: ", std::fixed, std::setprecision(1), fps, " lfps: ", lfps);
    }

    /* Update the deterministic timer, sleep if necessary and mix audio */
    detTimer.enterFrameBoundary();

    /* Send information to the game and notify for the beginning of the frame
     * boundary.
     */

    /* Send error messages */
    std::string alert;
    while (getAlertMsg(alert)) {
        sendMessage(MSGB_ALERT_MSG);
        sendString(alert);
    }

    /* Send framecount and internal time */
    sendMessage(MSGB_FRAMECOUNT_TIME);
    sendData(&framecount, sizeof(unsigned long));
    struct timespec ticks = detTimer.getTicks();
    sendData(&ticks, sizeof(struct timespec));

    /* Send GameInfo struct if needed */
    if (game_info.tosend) {
        sendMessage(MSGB_GAMEINFO);
        sendData(&game_info, sizeof(game_info));
        game_info.tosend = false;
    }

    /* Send fps and lfps values */
    sendMessage(MSGB_FPS);
    sendData(&fps, sizeof(float));
    sendData(&lfps, sizeof(float));

    /* Last message to send */
    sendMessage(MSGB_START_FRAMEBOUNDARY);

    /* Get ramwatches from the program */
    RenderHUD::resetWatches();

    int message = receiveMessage();
    while (message == MSGN_RAMWATCH) {
        std::string ramwatch = receiveString();
        RenderHUD::insertWatch(ramwatch);
        message = receiveMessage();
    }

    /*** Rendering ***/
    if (!drawFB)
        nondraw_framecount++;

    /* Update window title */
    if (!skipping_draw)
        WindowTitle::update(fps, lfps);

    /* Saving the screen pixels before drawing. This is done before rendering
     * the HUD, so that we can redraw with another HUD.
     */
    if (!skipping_draw) {
        if (drawFB && shared_config.save_screenpixels) {
            ScreenCapture::storePixels();
        }
    }

#ifdef LIBTAS_ENABLE_HUD
    if (!skipping_draw && shared_config.osd_encode) {
        hud.resetOffsets();
        if (shared_config.osd & SharedConfig::OSD_FRAMECOUNT) {
            hud.renderFrame(framecount);
            // hud.renderNonDrawFrame(nondraw_framecount);
        }
        if (shared_config.osd & SharedConfig::OSD_INPUTS)
            hud.renderInputs(ai);

        if (shared_config.osd & SharedConfig::OSD_MESSAGES)
            hud.renderMessages();

        if (shared_config.osd & SharedConfig::OSD_RAMWATCHES)
            hud.renderWatches();
    }
#endif

    /* Audio mixing is done above, so encode must be called after */
    /* Dumping audio and video */
    if (shared_config.av_dumping) {

        /* First, create the AVEncoder is needed */
        if (!avencoder) {
            debuglog(LCF_DUMP, "Start AV dumping on file ", AVEncoder::dumpfile);
            avencoder.reset(new AVEncoder());
        }

        /* Write the current frame */
        avencoder->encodeOneFrame(drawFB);
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

#ifdef LIBTAS_ENABLE_HUD
    if (!skipping_draw && !shared_config.osd_encode) {
        hud.resetOffsets();
        if (shared_config.osd & SharedConfig::OSD_FRAMECOUNT) {
            hud.renderFrame(framecount);
            // hud.renderNonDrawFrame(nondraw_framecount);
        }
        if (shared_config.osd & SharedConfig::OSD_INPUTS)
            hud.renderInputs(ai);

        if (shared_config.osd & SharedConfig::OSD_MESSAGES)
            hud.renderMessages();

        if (shared_config.osd & SharedConfig::OSD_RAMWATCHES)
            hud.renderWatches();
    }
#endif

    /* Actual draw command */
    if (!skipping_draw) {
        GlobalNoLog gnl;
        NATIVECALL(draw());
    }

    /* Receive messages from the program */
    #ifdef LIBTAS_ENABLE_HUD
        receive_messages(draw, hud);
    #else
        receive_messages(draw);
    #endif

    if (restore_screen) {
        if (!skipping_draw && drawFB && shared_config.save_screenpixels) {
            ScreenCapture::setPixels();
        }
    }

    /*** Process inputs and events ***/
    if ((game_info.video & GameInfo::SDL1) || (game_info.video & GameInfo::SDL2)) {
        /* Push native SDL events into our emulated event queue */
        pushNativeEvents();
    }

    /* Push generated events. This must be done after getting the new inputs. */
    generateKeyUpEvents();
    generateKeyDownEvents();
    if (framecount == 1)
        generateControllerAdded();
    generateControllerEvents();
    generateMouseMotionEvents();
    generateMouseButtonEvents();

    if ((game_info.keyboard & GameInfo::XEVENTS) || (game_info.mouse & GameInfo::XEVENTS)) {
        /* Sync Xlib event queue, so that we are sure the input events will be
         * pulled by the game on the next frame.
         */
        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (gameDisplays[i]) {
                XSync(gameDisplays[i], False);
            }
        }
    }

    /* Decide if we skip drawing the next frame because of fastforward.
     * It is stored in an extern so that we can disable opengl draws.
     */
    skipping_draw = skipDraw(fps);

    detTimer.exitFrameBoundary();

    debuglog(LCF_FRAME, "Leave frame boundary");
}

static void pushQuitEvent(void)
{
    if (game_info.video & GameInfo::SDL1) {
        SDL1::SDL_Event ev;
        ev.type = SDL1::SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }

    else if (game_info.video & GameInfo::SDL2) {
        SDL_Event ev;
        ev.type = SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }

    else {
        GlobalNoLog gnl;
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.window = gameXWindow;
        xev.xclient.format = 32;
        xev.xclient.data.l[1] = CurrentTime;

        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (gameDisplays[i]) {
                xev.xclient.message_type = XInternAtom(gameDisplays[i], "WM_PROTOCOLS", true);
                xev.xclient.data.l[0] = XInternAtom(gameDisplays[i], "WM_DELETE_WINDOW", False);
                OWNCALL(XSendEvent(gameDisplays[i], gameXWindow, False, NoEventMask, &xev));
            }
        }
    }
}


#ifdef LIBTAS_ENABLE_HUD
static void screen_redraw(std::function<void()> draw, RenderHUD& hud, AllInputs preview_ai)
#else
static void screen_redraw(std::function<void()> draw, AllInputs preview_ai)
#endif
{
    if (!skipping_draw && shared_config.save_screenpixels) {
        ScreenCapture::setPixels();

#ifdef LIBTAS_ENABLE_HUD
        hud.resetOffsets();
        if (shared_config.osd & SharedConfig::OSD_FRAMECOUNT) {
            hud.renderFrame(framecount);
            // hud.renderNonDrawFrame(nondraw_framecount);
        }
        if (shared_config.osd & SharedConfig::OSD_INPUTS) {
            hud.renderInputs(ai);
            hud.renderPreviewInputs(preview_ai);
        }

        if (shared_config.osd & SharedConfig::OSD_MESSAGES)
            hud.renderMessages();

        if (shared_config.osd & SharedConfig::OSD_RAMWATCHES)
            hud.renderWatches();
#endif

        NATIVECALL(draw());
    }
}

#ifdef LIBTAS_ENABLE_HUD
static void receive_messages(std::function<void()> draw, RenderHUD& hud)
#else
static void receive_messages(std::function<void()> draw)
#endif
{
    AllInputs preview_ai;
    preview_ai.emptyInputs();
    std::string savestatepath;
    int index;

    while (1)
    {
        int message = receiveMessage();
        std::string str;

        switch (message)
        {
            case MSGN_USERQUIT:
                pushQuitEvent();
                is_exiting = true;
                break;

            case MSGN_CONFIG:
                receiveData(&shared_config, sizeof(SharedConfig));
                break;

            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET | LCF_FRAME, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglog(LCF_SOCKET | LCF_FRAME, "File ", AVEncoder::dumpfile);
                receiveCString(AVEncoder::ffmpeg_options);
                break;

            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(AllInputs));
                break;

            case MSGN_EXPOSE:
#ifdef LIBTAS_ENABLE_HUD
                screen_redraw(draw, hud, preview_ai);
#else
                screen_redraw(draw, preview_ai);
#endif
                break;

            case MSGN_PREVIEW_INPUTS:
                receiveData(&preview_ai, sizeof(AllInputs));
#ifdef LIBTAS_ENABLE_HUD
                screen_redraw(draw, hud, preview_ai);
#else
                screen_redraw(draw, preview_ai);
#endif
                break;

            case MSGN_SAVESTATE_PATH:
                /* Get the parent savestate path */
                savestatepath = receiveString();
                Checkpoint::setSavestatePath(savestatepath);
                break;

            case MSGN_SAVESTATE_INDEX:
                /* Get the parent savestate index */
                receiveData(&index, sizeof(int));
                Checkpoint::setSavestateIndex(index);
                break;

            case MSGN_PARENT_SAVESTATE_PATH:
                /* Get the parent savestate path */
                savestatepath = receiveString();
                Checkpoint::setParentSavestatePath(savestatepath);
                break;

            case MSGN_PARENT_SAVESTATE_INDEX:
                /* Get the parent savestate index */
                receiveData(&index, sizeof(int));
                Checkpoint::setParentSavestateIndex(index);
                break;

            case MSGN_SAVESTATE:
                ThreadManager::checkpoint();

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
                    sendData(&framecount, sizeof(unsigned long));
                    struct timespec ticks = detTimer.getTicks();
                    sendData(&ticks, sizeof(struct timespec));

                    /* Screen should have changed after loading */
                    ScreenCapture::setPixels();
                }

                break;

            case MSGN_LOADSTATE:
                ThreadManager::restore();

                /* If restoring failed, we return here. We still send the
                 * frame count and time because the program will pull a
                 * message in either case.
                 */
                sendMessage(MSGB_FRAMECOUNT_TIME);
                sendData(&framecount, sizeof(unsigned long));
                {
                    struct timespec ticks = detTimer.getTicks();
                    sendData(&ticks, sizeof(struct timespec));
                }

                break;

            case MSGN_STOP_ENCODE:
                if (avencoder) {
                    debuglog(LCF_DUMP, "Stop AV dumping");
                    avencoder.reset(nullptr);
                    shared_config.av_dumping = false;

                    /* Update title without changing fps */
                    WindowTitle::update(-1, -1);
                }
                break;

            case MSGN_OSD_MSG:
                RenderHUD::insertMessage(receiveString().c_str());
#ifdef LIBTAS_ENABLE_HUD
                screen_redraw(draw, hud, preview_ai);
#else
                screen_redraw(draw, preview_ai);
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
