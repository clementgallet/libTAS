/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdlevents.h"
#include "sdlversion.h"
#include "SDLEventQueue.h"
#include "sdldynapi.h"

#include "logging.h"
#include "hook.h"
#include "general/sleepwrappers.h"
#include "GlobalState.h"
#include "inputs/sdlkeyboard.h"
#include "global.h"

namespace libtas {

static_assert(static_cast<int>(sdl1::SDL_ADDEVENT) == static_cast<int>(sdl2::SDL_ADDEVENT), "SDL1 and SDL2 event action values must be the same");
static_assert(static_cast<int>(sdl1::SDL_PEEKEVENT) == static_cast<int>(sdl2::SDL_PEEKEVENT), "SDL1 and SDL2 event action values must be the same");
static_assert(static_cast<int>(sdl1::SDL_GETEVENT) == static_cast<int>(sdl2::SDL_GETEVENT), "SDL1 and SDL2 event action values must be the same");

/* Return if the SDL 3 event must be passed to the game or be filtered */
static bool isBannedEvent(sdl3::SDL_Event *event)
{
    switch(event->type) {
        case sdl3::SDL_EVENT_KEY_DOWN:
        case sdl3::SDL_EVENT_KEY_UP:
        case sdl3::SDL_EVENT_MOUSE_MOTION:
        case sdl3::SDL_EVENT_MOUSE_BUTTON_DOWN:
        case sdl3::SDL_EVENT_MOUSE_BUTTON_UP:
        case sdl3::SDL_EVENT_MOUSE_WHEEL:
        case sdl3::SDL_EVENT_JOYSTICK_AXIS_MOTION:
        case sdl3::SDL_EVENT_JOYSTICK_BALL_MOTION:
        case sdl3::SDL_EVENT_JOYSTICK_HAT_MOTION:
        case sdl3::SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case sdl3::SDL_EVENT_JOYSTICK_BUTTON_UP:
        case sdl3::SDL_EVENT_JOYSTICK_ADDED:
        case sdl3::SDL_EVENT_JOYSTICK_REMOVED:
        case sdl3::SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
        case sdl3::SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
        case sdl3::SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case sdl3::SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case sdl3::SDL_EVENT_GAMEPAD_BUTTON_UP:
        case sdl3::SDL_EVENT_GAMEPAD_ADDED:
        case sdl3::SDL_EVENT_GAMEPAD_REMOVED:
        case sdl3::SDL_EVENT_GAMEPAD_REMAPPED:
        case sdl3::SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
        case sdl3::SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
        case sdl3::SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
        case sdl3::SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
        case sdl3::SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
        case sdl3::SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
        case sdl3::SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH:
        case sdl3::SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE:
        case sdl3::SDL_EVENT_FINGER_DOWN:
        case sdl3::SDL_EVENT_FINGER_UP:
        case sdl3::SDL_EVENT_FINGER_MOTION:
        case sdl3::SDL_EVENT_FINGER_CANCELED:
        case sdl3::SDL_EVENT_PINCH_BEGIN:
        case sdl3::SDL_EVENT_PINCH_UPDATE:
        case sdl3::SDL_EVENT_PINCH_END:
        case sdl3::SDL_EVENT_WINDOW_FOCUS_GAINED:
        case sdl3::SDL_EVENT_WINDOW_FOCUS_LOST:
        case sdl3::SDL_EVENT_WINDOW_SHOWN:
        case sdl3::SDL_EVENT_WINDOW_EXPOSED:
        case sdl3::SDL_EVENT_WINDOW_MOUSE_ENTER:
        case sdl3::SDL_EVENT_WINDOW_MOUSE_LEAVE:
            return true;
        default:
            return false;
    }
}

/* Return if the SDL 2 event must be passed to the game or be filtered */
static bool isBannedEvent(sdl2::SDL_Event *event)
{
    switch(event->type) {
        case sdl2::SDL_KEYDOWN:
        case sdl2::SDL_KEYUP:
        case sdl2::SDL_MOUSEMOTION:
        case sdl2::SDL_MOUSEBUTTONDOWN:
        case sdl2::SDL_MOUSEBUTTONUP:
        case sdl2::SDL_MOUSEWHEEL:
        case sdl2::SDL_JOYAXISMOTION:
        case sdl2::SDL_JOYBALLMOTION:
        case sdl2::SDL_JOYHATMOTION:
        case sdl2::SDL_JOYBUTTONDOWN:
        case sdl2::SDL_JOYBUTTONUP:
        case sdl2::SDL_JOYDEVICEADDED:
        case sdl2::SDL_JOYDEVICEREMOVED:
        case sdl2::SDL_CONTROLLERAXISMOTION:
        case sdl2::SDL_CONTROLLERBUTTONDOWN:
        case sdl2::SDL_CONTROLLERBUTTONUP:
        case sdl2::SDL_CONTROLLERDEVICEADDED:
        case sdl2::SDL_CONTROLLERDEVICEREMOVED:
        case sdl2::SDL_CONTROLLERDEVICEREMAPPED:
            return true;
        case sdl2::SDL_WINDOWEVENT:
            switch (event->window.event) {
                case sdl2::SDL_WINDOWEVENT_FOCUS_GAINED:
                case sdl2::SDL_WINDOWEVENT_FOCUS_LOST:
                case sdl2::SDL_WINDOWEVENT_SHOWN:
                case sdl2::SDL_WINDOWEVENT_EXPOSED:
                case sdl2::SDL_WINDOWEVENT_ENTER:
                case sdl2::SDL_WINDOWEVENT_LEAVE:
                case sdl2::SDL_WINDOWEVENT_TAKE_FOCUS:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

/* Return if the SDL 1 event must be passed to the game or be filtered */
static bool isBannedEvent(sdl1::SDL_Event *event)
{
    switch(event->type) {
        case sdl1::SDL_ACTIVEEVENT:
        case sdl1::SDL_KEYDOWN:
        case sdl1::SDL_KEYUP:
        case sdl1::SDL_MOUSEMOTION:
        case sdl1::SDL_MOUSEBUTTONDOWN:
        case sdl1::SDL_MOUSEBUTTONUP:
        case sdl1::SDL_JOYAXISMOTION:
        case sdl1::SDL_JOYBALLMOTION:
        case sdl1::SDL_JOYHATMOTION:
        case sdl1::SDL_JOYBUTTONDOWN:
        case sdl1::SDL_JOYBUTTONUP:
            return true;
        default:
            return false;
    }
}

void pushNativeSDLEvents(void)
{
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    /* SDL_PumpEvents may call SDL_GetTicks() a lot, and we don't want to
     * advance the timer because of that, so we make it untrack
     */
    GlobalOwnCode toc;

    // NOLOGCALL(ORIG_SDL2_CALL(SDL_PumpEvents, ()));

    /* We use SDL_PeepEvents() for gathering events from the SDL queue,
     * as it is the native function of getting events.
     * i.e. all other functions call this function internally.
     */
    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        sdl1::SDL_Event ev;
        while (ORIG_SDL2_CALL(SDL_PeepEvents, (reinterpret_cast<sdl2::SDL_Event*>(&ev), 1, sdl2::SDL_GETEVENT, sdl1::SDL_ALLEVENTS, 0))) {
            if (ev.type == sdl1::SDL_QUIT) {
                Global::is_exiting = true;
            }
            if (! isBannedEvent(&ev))
                sdlEventQueue.insert(&ev);
        }
    }
    
    if (SDLver == 2) {
        sdl2::SDL_Event ev;
        while (ORIG_SDL2_CALL(SDL_PeepEvents, (&ev, 1, sdl2::SDL_GETEVENT, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT))) {
            if (ev.type == sdl2::SDL_QUIT) {
                Global::is_exiting = true;
            }
            if (! isBannedEvent(&ev))
                sdlEventQueue.insert(&ev);
        }
    }
}

/* Override */ void SDL_PumpEvents(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);
    
    /* Update the internal keyboard array */
    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        NOLOGCALL(SDL_GetKeyState(nullptr));
    }

    if (SDLver == 2) {
        NOLOGCALL(SDL_GetKeyboardState(nullptr));
    }
    
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_PumpEvents, ());
    }
}

/* Override */ int SDL_PeepEvents(sdl2::SDL_Event* events2, int numevents, sdl2::SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_PeepEvents, (events2, numevents, action, minType, maxType));
    }

    /* We need to use a function signature with variable arguments,
     * because SDL 1.2 and SDL 2/3 provide a different function with the same name.
     * SDL 1.2 is int SDL_PeepEvents(SDL1_Event *events, int numevents, SDL_eventaction action, Uint32 mask);
     * SDL 2/3 is int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
     */

    Uint32 mask;
    sdl1::SDL_Event* events1 = nullptr;
    sdl3::SDL_Event* events3 = nullptr;

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        mask = minType;
        events1 = reinterpret_cast<sdl1::SDL_Event*>(events2);
    }
    if (SDLver == 3) {
        events3 = reinterpret_cast<sdl3::SDL_Event*>(events2);
    }

    static_assert(static_cast<int>(sdl2::SDL_ADDEVENT) == static_cast<int>(sdl3::SDL_ADDEVENT), "SDL2 and SDL3 event action values must be the same");
    static_assert(static_cast<int>(sdl2::SDL_PEEKEVENT) == static_cast<int>(sdl3::SDL_PEEKEVENT), "SDL2 and SDL3 event action values must be the same");
    static_assert(static_cast<int>(sdl2::SDL_GETEVENT) == static_cast<int>(sdl3::SDL_GETEVENT), "SDL2 and SDL3 event action values must be the same");

    int nevents = 0;
    switch (action) {
        case sdl2::SDL_ADDEVENT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "The game wants to add %d events", numevents);
            if ((numevents > 0) && (events2 == nullptr))
                return -1;
            for (int i=0; i<numevents; i++) {
                if (SDLver == 1) {
                    if (! isBannedEvent(&events1[i])) {
                        int ret = sdlEventQueue.insert(&events1[i]);
                        if (ret == 0) nevents++;
                    }
                    else {
                        nevents++;
                    }
                }
                if (SDLver == 2) {
                    if (! isBannedEvent(&events2[i])) {
                        int ret = sdlEventQueue.insert(&events2[i]);
                        /* Return -1 if the queue is fulled */
                        if (ret == -1) return -1;
                        else nevents += ret;
                    }
                    else {
                        nevents++;
                    }
                }
                if (SDLver == 3) {
                    if (! isBannedEvent(&events3[i])) {
                        int ret = sdlEventQueue.insert(&events3[i]);
                        /* Return -1 if the queue is fulled */
                        if (ret == -1) return -1;
                        else nevents += ret;
                    }
                    else {
                        nevents++;
                    }
                }
            }
            return nevents;
        case sdl2::SDL_PEEKEVENT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "The game wants to peek at %d events", numevents);
            if (SDLver == 1)
                return sdlEventQueue.pop(events1, numevents, mask, false);
            if (SDLver == 2)
                return sdlEventQueue.pop(events2, numevents, minType, maxType, false);
            if (SDLver == 3)
                return sdlEventQueue.pop(events3, numevents, minType, maxType, false);
            break;
        case sdl2::SDL_GETEVENT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "The game wants to get %d events", numevents);
            if (SDLver == 1)
                return sdlEventQueue.pop(events1, numevents, mask, true);
            if (SDLver == 2)
                return sdlEventQueue.pop(events2, numevents, minType, maxType, true);
            if (SDLver == 3)
                return sdlEventQueue.pop(events3, numevents, minType, maxType, true);
            break;
    }

    return 0;
}

/* Override */ int SDL_PollEvent(sdl2::SDL_Event *event)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        int ret = ORIG_SDL23_CALL(SDL_PollEvent, (event));
        if (event && (ret == 1))
            logEvent(event);
        return ret;
    }
    
    /* From SDL2/3 code, this function calls SDL_PumpEvents at the beginning */
    // NOLOGCALL(ORIG_SDL2_CALL(SDL_PumpEvents, ()));

    int SDLver = get_sdlversion();
    if (event) {
        /* Fetch one event with update using our helper function */
        if (SDLver == 1)
            return sdlEventQueue.pop(reinterpret_cast<sdl1::SDL_Event*>(event), 1, sdl1::SDL_ALLEVENTS, true);
        if (SDLver == 2)
            return sdlEventQueue.pop(event, 1, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT, true);
        if (SDLver == 3)
            return sdlEventQueue.pop(reinterpret_cast<sdl3::SDL_Event*>(event), 1, sdl3::SDL_EVENT_FIRST, sdl3::SDL_EVENT_LAST, true);
    } else {
        /*
         * In the case the event pointer is NULL, SDL doc says to
         * return 1 if there is an event in the queue and 0 of not,
         * without updating the queue
         */
        if (SDLver == 1) {
            sdl1::SDL_Event ev1;
            return sdlEventQueue.pop(&ev1, 1, sdl1::SDL_ALLEVENTS, false);
        }
        if (SDLver == 2) {
            sdl2::SDL_Event ev;
            return sdlEventQueue.pop(&ev, 1, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT, false);
        }
        if (SDLver == 3) {
            sdl3::SDL_Event ev;
            return sdlEventQueue.pop(&ev, 1, sdl3::SDL_EVENT_FIRST, sdl3::SDL_EVENT_LAST, false);
        }
    }
    return -1;
}

/* Override */ SDL_bool SDL_HasEvent(Uint32 type)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_HasEvent, (type));
    }

    return SDL_HasEvents(type, type);
}

/* Override */ SDL_bool SDL_HasEvents(Uint32 minType, Uint32 maxType)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_HasEvents, (minType, maxType));
    }

    int SDLver = get_sdlversion();

    /* Try to get one event without updating, and return if we got one */
    if (SDLver == 2) {
        sdl2::SDL_Event ev;
        return sdlEventQueue.pop(&ev, 1, minType, maxType, false) ? SDL_TRUE : SDL_FALSE;
    }
    if (SDLver == 3) {
        sdl3::SDL_Event ev;
        return sdlEventQueue.pop(&ev, 1, minType, maxType, false) ? SDL_TRUE : SDL_FALSE;
    }

    return false;
}

/* Override */ void SDL_FlushEvent(Uint32 type)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_FlushEvent, (type));
    }

    return SDL_FlushEvents(type, type);
}

/* Override */ void SDL_FlushEvents(Uint32 minType, Uint32 maxType)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_FlushEvents, (minType, maxType));
    }

    sdlEventQueue.flush(minType, maxType);
}

/* Override */ int SDL_WaitEvent(sdl2::SDL_Event * event)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_WaitEvent, (event));
    }

    /* From SDL2/3 code, this function calls SDL_PumpEvents at the beginning */
    NOLOGCALL(SDL_PumpEvents());

    struct timespec mssleep = {0, 1000000};

    int SDLver = get_sdlversion();

    sdl2::SDL_Event ev;
    if (!event) event = &ev;

    if (SDLver == 2) {
        while (! sdlEventQueue.pop(event, 1, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT, true)) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
        }
    }
    if (SDLver == 3) {
        while (! sdlEventQueue.pop(reinterpret_cast<sdl3::SDL_Event*>(event), 1, sdl3::SDL_EVENT_FIRST, sdl3::SDL_EVENT_LAST, true)) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
        }
    }
    return 1;
}

/* Override */ int SDL_WaitEventTimeout(sdl2::SDL_Event * event, int timeout)
{
    LOGTRACE(LCF_SDL | LCF_EVENTS | LCF_TODO, "%s call with timeout %d", __func__, timeout);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_WaitEventTimeout, (event, timeout));
    }

    /* From SDL2/3 code, this function calls SDL_PumpEvents at the beginning */
    NOLOGCALL(SDL_PumpEvents());

    int t = 0;
    struct timespec mssleep = {0, 1000000};
    sdl2::SDL_Event ev;
    if (!event) event = &ev;

    int SDLver = get_sdlversion();

    if (SDLver == 2) {
        if (sdlEventQueue.pop(event, 1, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT, true))
            return 1;
        for (t=0; t<timeout; t++) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
            if (sdlEventQueue.pop(event, 1, sdl2::SDL_FIRSTEVENT, sdl2::SDL_LASTEVENT, true))
                break;
        }
    }
    if (SDLver == 3) {
        if (sdlEventQueue.pop(reinterpret_cast<sdl3::SDL_Event*>(event), 1, sdl3::SDL_EVENT_FIRST, sdl3::SDL_EVENT_LAST, true))
            return 1;
        for (t=0; t<timeout; t++) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
            if (sdlEventQueue.pop(reinterpret_cast<sdl3::SDL_Event*>(event), 1, sdl3::SDL_EVENT_FIRST, sdl3::SDL_EVENT_LAST, true))
                break;
        }
    }

    return (t<timeout);
}

/* Override */ int SDL_PushEvent(sdl2::SDL_Event * event)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (event == nullptr)
        return -1;

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_PushEvent, (event));
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        sdl1::SDL_Event* ev1 = reinterpret_cast<sdl1::SDL_Event*>(event);

        if (isBannedEvent(ev1))
            return 0;

        int ret = sdlEventQueue.insert(ev1);

        if (ev1->type == sdl1::SDL_QUIT) {
            Global::is_exiting = true;
        }

        return ret; // success
    }

    if (SDLver == 3) {
        sdl3::SDL_Event* ev3 = reinterpret_cast<sdl3::SDL_Event*>(event);
        if (isBannedEvent(ev3))
            return 0;

        int ret = sdlEventQueue.insert(ev3);

        if (ev3->type == sdl3::SDL_EVENT_QUIT) {
            /* SDL may be used only for controllers. In that case, exiting SDL does
            * not mean that the game is stopped */
            Uint32 init_flags = sdl3::SDL_WasInit(static_cast<sdl3::SDL_InitFlags>(0));
            if (init_flags & sdl3::SDL_INIT_VIDEO)
                Global::is_exiting = true;
        }

        return ret;
    }

    if (isBannedEvent(event))
        return 0;

    int ret = sdlEventQueue.insert(event);

    if (event->type == sdl2::SDL_QUIT) {
        /* SDL may be used only for controllers. In that case, exiting SDL does
        * not mean that the game is stopped */
        Uint32 init_flags = sdl2::SDL_WasInit(0);
        if (init_flags & sdl2::SDL_INIT_VIDEO)
            Global::is_exiting = true;
    }

    return ret;
}

/* Override */ void SDL_SetEventFilter(sdl2::SDL_EventFilter filter, void *userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_SetEventFilter, (filter, userdata));
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1)
        sdlEventQueue.setFilter(reinterpret_cast<sdl1::SDL_EventFilter>(filter));
    if (SDLver == 2)
        sdlEventQueue.setFilter(filter, userdata);
    if (SDLver == 3)
        sdlEventQueue.setFilter(reinterpret_cast<sdl3::SDL_EventFilter>(filter), userdata);
}

/* Override */ SDL_bool SDL_GetEventFilter(sdl2::SDL_EventFilter * filter, void **userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (filter)
        *filter = nullptr;
    if (userdata)
        *userdata = nullptr;

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_GetEventFilter, (filter, userdata));
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        LOG(LL_ERROR, LCF_SDL | LCF_EVENTS, "Not supported yet for SDL1");
        return SDL_FALSE;
        // return reinterpret_cast<SDL_bool>(sdlEventQueue.getFilter());
    }
    if (SDLver == 2) {
        if (sdlEventQueue.getFilter(filter, userdata))
            return SDL_TRUE;
        return SDL_FALSE;
    }
    if (SDLver == 3) {
        if (sdlEventQueue.getFilter(reinterpret_cast<sdl3::SDL_EventFilter*>(filter), userdata))
            return SDL_TRUE;
        return SDL_FALSE;
    }

    return SDL_FALSE;
}

/* Override */ void SDL_AddEventWatch(sdl2::SDL_EventFilter filter, void *userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_AddEventWatch, (filter, userdata));
    }

    int SDLver = get_sdlversion();
    if (SDLver == 2)
        sdlEventQueue.addWatch(filter, userdata);
    if (SDLver == 3)
        sdlEventQueue.addWatch(reinterpret_cast<sdl3::SDL_EventFilter>(filter), userdata);
}

/* Override */ void SDL_DelEventWatch(sdl2::SDL_EventFilter filter, void *userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL2_CALL(SDL_DelEventWatch, (filter, userdata));
    }

    sdlEventQueue.delWatch(filter, userdata);
}

/* Override */ void SDL_RemoveEventWatch(sdl3::SDL_EventFilter filter, void *userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL3_CALL(SDL_RemoveEventWatch, (filter, userdata));
    }

    sdlEventQueue.delWatch(filter, userdata);
}

/* Override */ void SDL_FilterEvents(sdl2::SDL_EventFilter filter, void *userdata)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL23_CALL(SDL_FilterEvents, (filter, userdata));
    }

    int SDLver = get_sdlversion();
    if (SDLver == 2)
        sdlEventQueue.applyFilter(filter, userdata);
    if (SDLver == 3)
        sdlEventQueue.applyFilter(reinterpret_cast<sdl3::SDL_EventFilter>(filter), userdata);
}

/* Override */ Uint8 SDL_EventState(Uint32 type, int state)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL2_CALL(SDL_EventState, (type, state));
    }

    int previousState = sdlEventQueue.isEnabled(type) ? SDL_ENABLE : SDL_DISABLE;
    switch (state) {
        case SDL_ENABLE:
            sdlEventQueue.enable(type);
            return previousState;
        case SDL_DISABLE:
            sdlEventQueue.disable(type);
            return previousState;
        case SDL_QUERY:
            return previousState;
    }
    return state;
}

/* Override */ void SDL_SetEventEnabled(Uint32 type, bool enabled)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL3_CALL(SDL_SetEventEnabled, (type, enabled));
    }

    if (enabled)
        sdlEventQueue.enable(type);
    else
        sdlEventQueue.disable(type);
}

/* Override */ bool SDL_EventEnabled(Uint32 type)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL3_CALL(SDL_EventEnabled, (type));
    }

    return sdlEventQueue.isEnabled(type);
}

/* Override */ Uint32 SDL_RegisterEvents(int numevents)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_EVENTS | LCF_TODO);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return ORIG_SDL2_CALL(SDL_RegisterEvents, (numevents));
    }

    return sdl2::SDL_USEREVENT;
}

void logEvent(sdl2::SDL_Event *event)
{
    switch(event->type) {
        case sdl2::SDL_KEYDOWN:
        case sdl2::SDL_KEYUP:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving KEYUP/KEYDOWN event with scancode %d and sym %d", (int)event->key.keysym.scancode, event->key.keysym.sym);
            break;

        case sdl2::SDL_QUIT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving QUIT event.");
            break;

        case sdl2::SDL_WINDOWEVENT:
            switch (event->window.event) {
                case sdl2::SDL_WINDOWEVENT_FOCUS_GAINED:
                    LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Window %d gained keyboard focus.", event->window.windowID);
                    break;
                case sdl2::SDL_WINDOWEVENT_FOCUS_LOST:
                    LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Window %d lost keyboard focus.", event->window.windowID);
                    break;
                case sdl2::SDL_WINDOWEVENT_CLOSE:
                    LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Window %d closed.", event->window.windowID);
                    break;
                default:
                    LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Window event %d", (int)event->window.event);
                    break;
            }
            break;

        case sdl2::SDL_TEXTEDITING:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving a keyboard text editing event.");
            break;

        case sdl2::SDL_TEXTINPUT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving a keyboard text input event %s", event->text.text);
            break;
            /*
               case SDL2::SDL_KEYMAPCHANGED:
               LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a keymap change event.");
               break;
               */
        case sdl2::SDL_MOUSEMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a mouse move event.");
            break;

        case sdl2::SDL_MOUSEBUTTONDOWN:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a mouse button press event.");
            break;

        case sdl2::SDL_MOUSEBUTTONUP:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a mouse button release event.");
            break;

        case sdl2::SDL_MOUSEWHEEL:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a mouse wheel event.");
            break;

        case sdl2::SDL_JOYAXISMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick axis motion event.");
            break;

        case sdl2::SDL_JOYBALLMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick trackball event.");
            break;

        case sdl2::SDL_JOYHATMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick hat position event.");
            break;

        case sdl2::SDL_JOYBUTTONDOWN:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick button press event.");
            break;

        case sdl2::SDL_JOYBUTTONUP:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick button release event.");
            break;

        case sdl2::SDL_JOYDEVICEADDED:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick connected event.");
            break;

        case sdl2::SDL_JOYDEVICEREMOVED:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a joystick disconnected event.");
            break;

        case sdl2::SDL_CONTROLLERAXISMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller axis motion event.");
            break;

        case sdl2::SDL_CONTROLLERBUTTONDOWN:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller button press event.");
            break;

        case sdl2::SDL_CONTROLLERBUTTONUP:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller button release event.");
            break;

        case sdl2::SDL_CONTROLLERDEVICEADDED:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller connected event.");
            break;

        case sdl2::SDL_CONTROLLERDEVICEREMOVED:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller disconnected event.");
            break;

        case sdl2::SDL_CONTROLLERDEVICEREMAPPED:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a controller mapping update event.");
            break;

        case sdl2::SDL_FINGERDOWN:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving an input device touch event.");
            break;

        case sdl2::SDL_FINGERUP:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving an input device release event.");
            break;

        case sdl2::SDL_FINGERMOTION:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving an input device drag event.");
            break;

        case sdl2::SDL_DOLLARGESTURE:
        case sdl2::SDL_DOLLARRECORD:
        case sdl2::SDL_MULTIGESTURE:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a gesture event.");
            break;

        case sdl2::SDL_CLIPBOARDUPDATE:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a clipboard update event.");
            break;

        case sdl2::SDL_DROPFILE:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a drag and drop event.");
            break;
            /*
               case SDL2::SDL_AUDIODEVICEADDED:
               LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a new audio device event.");
               break;

               case SDL2::SDL_AUDIODEVICEREMOVED:
               LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a audio device removal event.");
               break;
               */
        case sdl2::SDL_RENDER_TARGETS_RESET:
            //            case SDL2::SDL_RENDER_DEVICE_RESET:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a render event.");
            break;

        case sdl2::SDL_USEREVENT:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving a user-specified event.");
            break;

        default:
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS, "Receiving an unknown event: %d", event->type);
            break;
    }
}

}
