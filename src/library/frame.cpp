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

#include "frame.h"
#include "../shared/AllInputs.h"
#include "../shared/messages.h"
#include "global.h" // Global::shared_config
#include "inputs/inputs.h" // AllInputs ai object
#include "inputs/inputevents.h"
#include "../shared/sockethelpers.h"
#include "logging.h"
#include "GlobalState.h"
#include "DeterministicTimer.h"
#include "encoding/AVEncoder.h"
#include "sdl/sdlwindows.h"
#include "sdl/sdlevents.h"
#include <iomanip>
#include <stdint.h>
#include "timewrappers.h" // clock_gettime
#include "checkpoint/ThreadManager.h"
#include "checkpoint/SaveStateManager.h"
#include "checkpoint/Checkpoint.h"
#include "checkpoint/ThreadSync.h"
#include "ScreenCapture.h"
#include "WindowTitle.h"
#include "sdl/SDLEventQueue.h"
#include "BusyLoopDetection.h"
#include "audio/AudioContext.h"
#include "hook.h"
#include "GameHacks.h"
#include "PerfTimer.h"

#ifdef __unix__
#include "xlib/xevents.h"
#include "xlib/xdisplay.h" // x11::gameDisplays
#include "xcb/xcbevents.h"
#include "xlib/xatom.h"
#include "xlib/XlibEventQueueList.h"
#include "xlib/xwindows.h" // x11::gameXWindows
#endif

namespace libtas {

DECLARE_ORIG_POINTER(SDL_PumpEvents)

/* Frame counter */
uint64_t framecount = 0;

/* Store the number of nondraw frames */
static uint64_t nondraw_framecount = 0;

/* Did we do at least one savestate? */
static bool didASavestate = false;

static void receive_messages(std::function<void()> draw, RenderHUD& hud);

/* Compute real and logical fps */
static void computeFPS(float& fps, float& lfps)
{
    /* Do We have enough values to compute fps? */
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
        lastTicks[compute_counter] = detTimer.getTicks();

        uint64_t deltaFrames = framecount - lastFrame;

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
            if (Global::shared_config.fastforward) {
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
    if (!Global::shared_config.fastforward)
        return false;

    /* Don't skip if frame-advancing */
    if (!Global::shared_config.running)
        return false;

    /* Never skip a draw when encoding. */
    if (Global::shared_config.av_dumping)
        return false;

    /* Apply the fast-forward render setting */
    switch(Global::shared_config.fastforward_render) {
        case SharedConfig::FF_RENDER_NO:
            return true;
        case SharedConfig::FF_RENDER_ALL:
            return false;
        default:
            break;
    }

    /* Skipping frames in Vulkan currently results in softlock. */
    if (Global::game_info.video & GameInfo::VULKAN)
        return false;

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

static void sendFrameCountTime()
{
    /* Detect an error on the first send, and exit the game if so */
    int ret = sendMessage(MSGB_FRAMECOUNT_TIME);
    if (ret == -1)
        exit(1);
        
    sendData(&framecount, sizeof(uint64_t));
    struct timespec ticks = detTimer.getTicks(SharedConfig::TIMETYPE_UNTRACKED_MONOTONIC);
    uint64_t ticks_val = ticks.tv_sec;
    sendData(&ticks_val, sizeof(uint64_t));
    ticks_val = ticks.tv_nsec;
    sendData(&ticks_val, sizeof(uint64_t));
    ticks = detTimer.getTicks(SharedConfig::TIMETYPE_UNTRACKED_REALTIME);
    ticks_val = ticks.tv_sec;
    sendData(&ticks_val, sizeof(uint64_t));
    ticks_val = ticks.tv_nsec;
    sendData(&ticks_val, sizeof(uint64_t));
}

void frameBoundary(std::function<void()> draw, RenderHUD& hud)
{
    perfTimer.switchTimer(PerfTimer::FrameTimer);


    static float fps, lfps = 0;

    ThreadManager::setCheckpointThread();
    ThreadManager::setMainThread();

    /* Reset the busy loop detector */
    BusyLoopDetection::reset();

    /* Wait for events to be processed by the game */
#ifdef __unix__
    if (Global::shared_config.async_events & SharedConfig::ASYNC_XEVENTS_END)
        xlibEventQueueList.waitForEmpty();
#endif
    if (Global::shared_config.async_events & SharedConfig::ASYNC_SDLEVENTS_END)
        sdlEventQueue.waitForEmpty();

    if ((Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_WITNESS) && (framecount > 11) && (draw)) {
        ThreadSync::detWait();
    }

    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_CELESTE) {
        ThreadSync::detWait();
    }
    
    perfTimer.switchTimer(PerfTimer::RenderTimer);

    if (GameHacks::isUnity()) {
        ThreadSync::detWait();        
    }
    perfTimer.switchTimer(PerfTimer::FrameTimer);

    /* Update the deterministic timer, sleep if necessary */
    TimeHolder timeIncrement = detTimer.enterFrameBoundary();

    /* Mix audio, except if the game opened a loopback context */
    if (! audiocontext.isLoopback) {
        audiocontext.mixAllSources(timeIncrement);
    }

    /* If the game is exiting, dont process the frame boundary, just draw and exit */
    if (Global::is_exiting) {
        detTimer.flushDelay();

        if (draw)
            NATIVECALL(draw());

        /* Still push native events so that the game can exit properly */
        if ((Global::game_info.video & GameInfo::SDL1) || (Global::game_info.video & GameInfo::SDL2)) {
            pushNativeSDLEvents();
        }

#ifdef __unix__
        if (!(Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS)) {
            pushNativeXlibEvents();
            pushNativeXcbEvents();
        }
#endif

        detTimer.exitFrameBoundary();
        return;
    }

    /*** Update time ***/

    /* First, increase the frame count */
    ++framecount;

    /* Compute new FPS values */
    if (draw) {
        computeFPS(fps, lfps);
    }

    /* Send information to the game and notify for the beginning of the frame
     * boundary.
     */

    /* Other threads may send socket messages, so we lock the socket */
    lockSocket();

    /* Send framecount and internal time */    
    sendFrameCountTime();

    /* Send GameInfo struct if needed */
    if (Global::game_info.tosend) {
        sendMessage(MSGB_GAMEINFO);
        sendData(&Global::game_info, sizeof(Global::game_info));
        Global::game_info.tosend = false;
    }

    /* Send fps and lfps values */
    sendMessage(MSGB_FPS);
    sendData(&fps, sizeof(float));
    sendData(&lfps, sizeof(float));

    /* Notify the program that threads have changed, so that it can show it
     * and trigger a backtrack savestate */
    if (ThreadManager::hasThreadListChanged()) {
        sendMessage(MSGB_INVALIDATE_SAVESTATES);
        ThreadManager::resetThreadListChanged();
    }

    /* Send message if non-draw frame */
    if (!draw) {
        sendMessage(MSGB_NONDRAW_FRAME);
    }

    /* Last message to send */
    sendMessage(MSGB_START_FRAMEBOUNDARY);

    /* Reset ramwatches and lua drawings */
    RenderHUD::resetWatches();
    RenderHUD::resetLua();

    /* Receive messages from the program */
    int message = receiveMessage();
    
    while (message != MSGN_START_FRAMEBOUNDARY) {
        switch (message) {
        case MSGN_RAMWATCH:
        {
            /* Get ramwatch from the program */
            std::string ramwatch = receiveString();
            RenderHUD::insertWatch(ramwatch);
            break;
        }
        case MSGN_LUA_RESOLUTION:
        {
            int w, h;
            ScreenCapture::getDimensions(w, h);
            sendMessage(MSGB_LUA_RESOLUTION);
            sendData(&w, sizeof(int));
            sendData(&h, sizeof(int));
            break;
        }
        case MSGN_LUA_TEXT:
        {
            int x, y;
            receiveData(&x, sizeof(int));
            receiveData(&y, sizeof(int));
            std::string text = receiveString();
            uint32_t fg, bg;
            receiveData(&fg, sizeof(uint32_t));
            receiveData(&bg, sizeof(uint32_t));
            RenderHUD::insertLuaText(x, y, text, fg, bg);
            break;
        }
        case MSGN_LUA_PIXEL:
        {
            int x, y;
            receiveData(&x, sizeof(int));
            receiveData(&y, sizeof(int));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            RenderHUD::insertLuaPixel(x, y, color);
            break;
        }
        case MSGN_LUA_RECT:
        {
            int x, y, w, h, thickness;
            receiveData(&x, sizeof(int));
            receiveData(&y, sizeof(int));
            receiveData(&w, sizeof(int));
            receiveData(&h, sizeof(int));
            receiveData(&thickness, sizeof(int));
            uint32_t outline, fill;
            receiveData(&outline, sizeof(uint32_t));
            receiveData(&fill, sizeof(uint32_t));
            RenderHUD::insertLuaRect(x, y, w, h, thickness, outline, fill);
            break;
        }
        case MSGN_LUA_LINE:
        {
            int x0, y0, x1, y1;
            receiveData(&x0, sizeof(int));
            receiveData(&y0, sizeof(int));
            receiveData(&x1, sizeof(int));
            receiveData(&y1, sizeof(int));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            RenderHUD::insertLuaLine(x0, y0, x1, y1, color);
            break;
        }
        case MSGN_LUA_ELLIPSE:
        {
            int center_x, center_y, radius_x, radius_y;
            receiveData(&center_x, sizeof(int));
            receiveData(&center_y, sizeof(int));
            receiveData(&radius_x, sizeof(int));
            receiveData(&radius_y, sizeof(int));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            RenderHUD::insertLuaEllipse(center_x, center_y, radius_x, radius_y, color);
            break;
        }
        }
        message = receiveMessage();
    }

    /*** Rendering ***/
    if (!draw)
        nondraw_framecount++;

    /* Update window title */
    if (!Global::skipping_draw)
        WindowTitle::update(fps, lfps);

    /* If we want HUD to appear in encodes, we need to draw it before saving
     * the window surface/texture/etc. This has the small drawback that we
     * won't be able to remove HUD messages during that frame. */
    if (!Global::skipping_draw && draw && Global::shared_config.osd_encode) {
        AllInputs preview_ai;
        preview_ai.emptyInputs();
        hud.drawAll(framecount, nondraw_framecount, ai, preview_ai);
    }

    if (!Global::skipping_draw) {
        if (draw) {
            ScreenCapture::copyScreenToSurface();
        }
    }

    /* Audio mixing is done above, so encode must be called after */
    /* Dumping audio and video */
    if (Global::shared_config.av_dumping) {

        /* First, create the AVEncoder is needed */
        if (!avencoder) {
            debuglogstdio(LCF_DUMP, "Start AV dumping on file %s", AVEncoder::dumpfile);
            avencoder.reset(new AVEncoder());
        }

        /* Write the current frame */
        avencoder->encodeOneFrame(!!draw, timeIncrement);
    }
    else {
        /* If there is still an encoder object, it means we just stopped
         * encoding, so we must delete the encoder object.
         */
        if (avencoder) {
            debuglogstdio(LCF_DUMP, "Stop AV dumping");
            avencoder.reset(nullptr);
        }
    }

    if (!Global::skipping_draw && draw && !Global::shared_config.osd_encode) {
        AllInputs preview_ai;
        preview_ai.emptyInputs();
        hud.drawAll(framecount, nondraw_framecount, ai, preview_ai);
    }

    /* Actual draw command */
    if (!Global::skipping_draw && draw) {
        GlobalNoLog gnl;
        perfTimer.switchTimer(PerfTimer::RenderTimer);
        NATIVECALL(draw());
        perfTimer.switchTimer(PerfTimer::FrameTimer);
    }

    /* Receive messages from the program */
    receive_messages(draw, hud);

    /* No more socket messages here, unlocking the socket. */
    unlockSocket();

    /* Some methods of drawing on screen don't always update the full screen.
     * Our current screen may be dirty with OSD, so in that case, we must
     * restore the screen to its original content so that the next frame will
     * be correct.
     * It is also the case for double buffer draw methods when the game does
     * not clean the back buffer.
     */
    if (!Global::skipping_draw && draw) {
        ScreenCapture::restoreScreenState();
    }

    /*** Process inputs and events ***/

    /* This part may disappear entirely if we manage to completely emulate
     * the event system. For now, we push some native events that the game might
     * expect to prevent some softlocks or other unexpected behaviors.
     */
    if ((Global::game_info.video & GameInfo::SDL1) || (Global::game_info.video & GameInfo::SDL2)) {
        /* Push native SDL events into our emulated event queue */
        pushNativeSDLEvents();
    }

#ifdef __unix__
    if (!(Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS)) {
        pushNativeXlibEvents();
        pushNativeXcbEvents();
    }
#endif

    /* Update game inputs based on current and previous inputs. This must be
     * done after getting the new inputs (obviously) and before pushing events,
     * because they used the new game inputs. */
    updateGameInputs();

#ifdef __unix__
    /* Reset the empty state of each xevent queue, for async event handling */
    if (Global::shared_config.async_events & (SharedConfig::ASYNC_XEVENTS_BEG | SharedConfig::ASYNC_XEVENTS_END)) {
        xlibEventQueueList.lock();
        xlibEventQueueList.resetEmpty();
    }
#endif

    /* Reset the empty state of the SDL queue, for async event handling */
    if (Global::shared_config.async_events & (SharedConfig::ASYNC_SDLEVENTS_BEG | SharedConfig::ASYNC_SDLEVENTS_END)) {
        sdlEventQueue.mutex.lock();
        sdlEventQueue.resetEmpty();
    }

    /* Push generated events. This must be done after getting the new inputs. */
    if (!(Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS)) {
        generateKeyUpEvents();
        generateKeyDownEvents();
        generateControllerAdded();
        generateControllerEvents();
        generateMouseMotionEvents();
        generateMouseButtonEvents();
    }

#ifdef __unix__
    if (Global::shared_config.async_events & (SharedConfig::ASYNC_XEVENTS_BEG | SharedConfig::ASYNC_XEVENTS_END)) {
        xlibEventQueueList.unlock();
    }
#endif

    if (Global::shared_config.async_events & (SharedConfig::ASYNC_SDLEVENTS_BEG | SharedConfig::ASYNC_SDLEVENTS_END)) {
        sdlEventQueue.mutex.unlock();
    }

    /* Wait for evdev and jsdev events to be processed by the game, in case of async event handling */
    syncControllerEvents();

    /* Wait for events to be processed by the game */
#ifdef __unix__
    if (Global::shared_config.async_events & SharedConfig::ASYNC_XEVENTS_BEG)
        xlibEventQueueList.waitForEmpty();
#endif

    if (Global::shared_config.async_events & SharedConfig::ASYNC_SDLEVENTS_BEG)
        sdlEventQueue.waitForEmpty();

    // ThreadSync::detSignalGlobal(0);
    // ThreadSync::detWaitGlobal(1);

    /* Decide if we skip drawing the next frame because of fastforward.
     * It is stored in an extern so that we can disable opengl draws.
     */
    Global::skipping_draw = skipDraw(fps);

    detTimer.exitFrameBoundary();

    perfTimer.switchTimer(PerfTimer::GameTimer);
    
    // if ((framecount % 10000) == 9999)
    //     perfTimer.print();
}

static void pushQuitEvent(void)
{
    if (Global::game_info.video & GameInfo::SDL1) {
        SDL1::SDL_Event ev;
        ev.type = SDL1::SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }

    if (Global::game_info.video & GameInfo::SDL2) {
        SDL_Event ev;
        ev.type = SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }

#ifdef __unix__
    if (x11::gameXWindows.empty())
        return;

    GlobalNoLog gnl;
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.window = x11::gameXWindows.front();
    xev.xclient.format = 32;
    xev.xclient.data.l[1] = CurrentTime;

    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i]) {
            xev.xclient.message_type = x11_atom(WM_PROTOCOLS);
            xev.xclient.data.l[0] = x11_atom(WM_DELETE_WINDOW);
            NATIVECALL(XSendEvent(x11::gameDisplays[i], x11::gameXWindows.front(), False, NoEventMask, &xev));
            NATIVECALL(XSync(x11::gameDisplays[i], false));
        }
    }
#endif
}


static void screen_redraw(std::function<void()> draw, RenderHUD& hud, AllInputs preview_ai)
{
    if (!Global::skipping_draw && draw) {
        ScreenCapture::copySurfaceToScreen();

        hud.drawAll(framecount, nondraw_framecount, ai, preview_ai);

        GlobalNoLog gnl;
        NATIVECALL(draw());
    }
}

static void receive_messages(std::function<void()> draw, RenderHUD& hud)
{
    AllInputs preview_ai;
    preview_ai.emptyInputs();
    std::string savestatepath;
    int slot;

    /* Catch dead children spawned for state saving */
    while (1) {
        int slot = SaveStateManager::waitChild();
        if (slot < 0) break;
        std::string msg = "State ";
        msg += std::to_string(slot);
        msg += " saved";
        RenderHUD::insertMessage(msg.c_str());
        screen_redraw(draw, hud, preview_ai);
    }

    while (1)
    {
        int message = receiveMessageNonBlocking();
        if (message < 0) {
#ifdef __unix__
            /* We need to answer to ping messages from the window manager,
             * otherwise the game will appear as unresponsive. */
            pushNativeXlibEvents();
            pushNativeXcbEvents();
#elif defined(__APPLE__) && defined(__MACH__)
            /* We need to poll events, otherwise the game appears as non-responsive.
             * TODO: Put this at appropriate place */
            if ((Global::game_info.video & GameInfo::SDL1) || (Global::game_info.video & GameInfo::SDL2)) {
                LINK_NAMESPACE_SDLX(SDL_PumpEvents);
                orig::SDL_PumpEvents();
            }
#endif
            
            /* Resume execution if game is exiting */
            if (Global::is_exiting)
                return;
            
            /* We only sleep if the game is in fast-forward, so that we don't
             * impact its performance. */
            if (! Global::shared_config.fastforward) {
                perfTimer.switchTimer(PerfTimer::IdleTimer);
                NATIVECALL(usleep(100));
                perfTimer.switchTimer(PerfTimer::FrameTimer);                
            }

            /* Catch dead children spawned for state saving */
            while (1) {
                int slot = SaveStateManager::waitChild();
                if (slot < 0) break;
                std::string msg = "State ";
                msg += std::to_string(slot);
                msg += " saved";
                RenderHUD::insertMessage(msg.c_str());
                screen_redraw(draw, hud, preview_ai);
            }
        }
        int status;
        std::string str;
        switch (message)
        {
            case -1:
                break;
            case MSGN_USERQUIT:
                pushQuitEvent();
                Global::is_exiting = true;
                break;

            case MSGN_CONFIG:
                receiveData(&Global::shared_config, sizeof(SharedConfig));
                break;

            case MSGN_DUMP_FILE:
                debuglogstdio(LCF_SOCKET, "Receiving dump filename");
                receiveCString(AVEncoder::dumpfile);
                debuglogstdio(LCF_SOCKET, "File %s", AVEncoder::dumpfile);
                receiveCString(AVEncoder::ffmpeg_options);
                break;

            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(AllInputs));
                /* Update framerate if necessary (do we actually need to?) */
                if (Global::shared_config.variable_framerate) {
                    Global::shared_config.framerate_num = ai.framerate_num;
                    Global::shared_config.framerate_den = ai.framerate_den;
                }
                /* Set new realtime value */
                if (ai.realtime_sec)
                    detTimer.setRealTime(ai.realtime_sec, ai.realtime_nsec);
                
                break;

            case MSGN_EXPOSE:
                screen_redraw(draw, hud, preview_ai);
                break;

            case MSGN_PREVIEW_INPUTS:
                receiveData(&preview_ai, sizeof(AllInputs));
                screen_redraw(draw, hud, preview_ai);
                break;

            case MSGN_SAVESTATE_PATH:
                /* Get the savestate path */
                savestatepath = receiveString();
                Checkpoint::setSavestatePath(savestatepath);
                break;

            case MSGN_SAVESTATE_INDEX:
                /* Get the savestate index */
                receiveData(&slot, sizeof(int));
                Checkpoint::setSavestateIndex(slot);
                break;

            case MSGN_SAVESTATE:
                status = SaveStateManager::checkpoint(slot);

                if (status == 0) {
                    /* Current savestate is now the parent savestate */
                    Checkpoint::setCurrentToParent();

                    /* We did at least one savestate, used for backtrack savestate */
                    didASavestate = true;
                }

                SaveStateManager::printError(status);

                /* Don't forget that when we load a savestate, the game continues
                 * from here and not from SaveStateManager::restore() under.
                 */
                if (SaveStateManager::isLoading()) {
                    /* Tell the program that the loading succeeded */
                    sendMessage(MSGB_LOADING_SUCCEEDED);

                    /* After loading, the game and the program no longer store
                     * the same information, so they must communicate to be
                     * synced again.
                     */

                    /* We receive the shared config struct */
                    message = receiveMessage();
                    MYASSERT(message == MSGN_CONFIG)
                    receiveData(&Global::shared_config, sizeof(SharedConfig));

                    /* We must send again the frame count and time because it
                     * probably has changed.
                     */
                    sendFrameCountTime();

                    /* Screen should have changed after loading */
                    screen_redraw(draw, hud, preview_ai);
                }
                else if (status == 0) {
                    /* Tell the program that the saving succeeded */
                    sendMessage(MSGB_SAVING_SUCCEEDED);

                    /* Print the successful message, unless we are saving in a fork */
                    if (!(Global::shared_config.savestate_settings & SharedConfig::SS_FORK)) {
                        if (Global::shared_config.osd & SharedConfig::OSD_MESSAGES) {
                            std::string msg;
                            msg = "State ";
                            msg += std::to_string(slot);
                            msg += " saved";
                            RenderHUD::insertMessage(msg.c_str());
                            screen_redraw(draw, hud, preview_ai);
                        }
                    }

                }
                else {
                    /* A bit hackish, we must send something */
                    sendMessage(-1);
                }

                break;

            case MSGN_LOADSTATE:
                status = SaveStateManager::restore(slot);

                SaveStateManager::printError(status);

                /* If restoring failed, we return here. We still send the
                 * frame count and time because the program will pull a
                 * message in either case.
                 */
                sendFrameCountTime();
                break;

            case MSGN_STOP_ENCODE:
                if (avencoder) {
                    debuglogstdio(LCF_DUMP, "Stop AV dumping");
                    avencoder.reset(nullptr);
                    Global::shared_config.av_dumping = false;

                    /* Update title without changing fps */
                    WindowTitle::update(-1, -1);
                }
                break;

            case MSGN_OSD_MSG:
                RenderHUD::insertMessage(receiveString().c_str());
                screen_redraw(draw, hud, preview_ai);
                break;

            case MSGN_END_FRAMEBOUNDARY:
                return;

            default:
                debuglogstdio(LCF_ERROR | LCF_SOCKET, "Unknown message received");
                return;
        }
    }
}

}
