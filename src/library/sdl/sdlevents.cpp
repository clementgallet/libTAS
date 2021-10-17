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

#include "sdlevents.h"
#include "../logging.h"
#include "../hook.h"
#include "sdlversion.h"
#include "SDLEventQueue.h"
#include "../sleepwrappers.h"
#include "../GlobalState.h"
#include "../inputs/sdlkeyboard.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_PumpEvents)
DECLARE_ORIG_POINTER(SDL_PeepEvents)

DECLARE_ORIG_POINTER(SDL_PollEvent)
DECLARE_ORIG_POINTER(SDL_HasEvent)
DECLARE_ORIG_POINTER(SDL_HasEvents)
DECLARE_ORIG_POINTER(SDL_FlushEvent)
DECLARE_ORIG_POINTER(SDL_FlushEvents)
DECLARE_ORIG_POINTER(SDL_WaitEvent)
DECLARE_ORIG_POINTER(SDL_WaitEventTimeout)
DECLARE_ORIG_POINTER(SDL_PushEvent)
DECLARE_ORIG_POINTER(SDL_SetEventFilter)
DECLARE_ORIG_POINTER(SDL_GetEventFilter)
DECLARE_ORIG_POINTER(SDL_AddEventWatch)
DECLARE_ORIG_POINTER(SDL_DelEventWatch)
DECLARE_ORIG_POINTER(SDL_FilterEvents)
DECLARE_ORIG_POINTER(SDL_EventState)
DECLARE_ORIG_POINTER(SDL_RegisterEvents)

/* Return if the SDL 2 event must be passed to the game or be filtered */
static bool isBannedEvent(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_JOYAXISMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            return true;
        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_EXPOSED:
                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_LEAVE:
                case SDL_WINDOWEVENT_TAKE_FOCUS:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

/* Return if the SDL 1 event must be passed to the game or be filtered */
static bool isBannedEvent(SDL1::SDL_Event *event)
{
    switch(event->type) {
        case SDL1::SDL_ACTIVEEVENT:
        case SDL1::SDL_KEYDOWN:
        case SDL1::SDL_KEYUP:
        case SDL1::SDL_MOUSEMOTION:
        case SDL1::SDL_MOUSEBUTTONDOWN:
        case SDL1::SDL_MOUSEBUTTONUP:
        case SDL1::SDL_JOYAXISMOTION:
        case SDL1::SDL_JOYBALLMOTION:
        case SDL1::SDL_JOYHATMOTION:
        case SDL1::SDL_JOYBUTTONDOWN:
        case SDL1::SDL_JOYBUTTONUP:
            return true;
        default:
            return false;
    }
}

void pushNativeSDLEvents(void)
{
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    LINK_NAMESPACE_SDLX(SDL_PeepEvents);
    LINK_NAMESPACE_SDLX(SDL_PumpEvents);

    /* SDL_PumpEvents may call SDL_GetTicks() a lot, and we don't want to
     * advance the timer because of that, so we make it untrack
     */
    GlobalOwnCode toc;

    NOLOGCALL(orig::SDL_PumpEvents());

    /* We use SDL_PeepEvents() for gathering events from the SDL queue,
     * as it is the native function of getting events.
     * i.e. all other functions call this function internally.
     */
    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        SDL1::SDL_Event ev;
        while (orig::SDL_PeepEvents(reinterpret_cast<SDL_Event*>(&ev), 1, SDL_GETEVENT, SDL1::SDL_ALLEVENTS, 0)) {
            if (ev.type == SDL1::SDL_QUIT) {
                is_exiting = true;
            }
            if (! isBannedEvent(&ev))
                sdlEventQueue.insert(&ev);
        }
    }

    if (SDLver == 2) {
        SDL_Event ev;
        while (orig::SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            if (ev.type == SDL_QUIT) {
                is_exiting = true;
            }
            if (! isBannedEvent(&ev))
                sdlEventQueue.insert(&ev);
        }
    }
}

/* Override */ void SDL_PumpEvents(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    
    /* Update the internal keyboard array */
    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        NOLOGCALL(SDL_GetKeyState(nullptr));
    }

    if (SDLver == 2) {
        NOLOGCALL(SDL_GetKeyboardState(nullptr));
    }
    
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_PumpEvents);
        return orig::SDL_PumpEvents();
    }
}

/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_PeepEvents);
        return orig::SDL_PeepEvents(events, numevents, action, minType, maxType);
    }

    /* We need to use a function signature with variable arguments,
     * because SDL 1.2 and SDL 2 provide a different function with the same name.
     * SDL 1.2 is int SDL_PeepEvents(SDL1_Event *events, int numevents, SDL_eventaction action, Uint32 mask);
     * SDL 2   is int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
     */

    Uint32 mask;
    SDL1::SDL_Event* events1 = nullptr;

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        mask = minType;
        events1 = reinterpret_cast<SDL1::SDL_Event*>(events);
    }

    int nevents = 0;
    switch (action) {
        case SDL_ADDEVENT:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "The game wants to add %d events", numevents);
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
                    if (! isBannedEvent(&events[i])) {
                        int ret = sdlEventQueue.insert(&events[i]);
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
        case SDL_PEEKEVENT:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "The game wants to peek at %d events", numevents);
            if (SDLver == 1)
                return sdlEventQueue.pop(events1, numevents, mask, false);
            if (SDLver == 2)
                return sdlEventQueue.pop(events, numevents, minType, maxType, false);
            break;
        case SDL_GETEVENT:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "The game wants to get %d events", numevents);
            if (SDLver == 1)
                return sdlEventQueue.pop(events1, numevents, mask, true);
            if (SDLver == 2)
                return sdlEventQueue.pop(events, numevents, minType, maxType, true);
            break;
    }

    return 0;
}

/* Override */ int SDL_PollEvent(SDL_Event *event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_PollEvent);
        int ret = orig::SDL_PollEvent(event);
        if (event && (ret == 1))
            logEvent(event);
        return ret;
    }
    
    /* From SDL2 code, this function calls SDL_PumpEvents at the beginning */
    NOLOGCALL(SDL_PumpEvents());

    int SDLver = get_sdlversion();
    if (event) {
        /* Fetch one event with update using our helper function */
        if (SDLver == 1)
            return sdlEventQueue.pop(reinterpret_cast<SDL1::SDL_Event*>(event), 1, SDL1::SDL_ALLEVENTS, true);
        if (SDLver == 2)
            return sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true);
    } else {
        /*
         * In the case the event pointer is NULL, SDL doc says to
         * return 1 if there is an event in the queue and 0 of not,
         * without updating the queue
         */
        if (SDLver == 1) {
            SDL1::SDL_Event ev1;
            return sdlEventQueue.pop(&ev1, 1, SDL1::SDL_ALLEVENTS, false);
        }
        if (SDLver == 2) {
            SDL_Event ev;
            return sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false);
        }
    }
    return -1;
}

/* Override */ SDL_bool SDL_HasEvent(Uint32 type)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_HasEvent);
        return orig::SDL_HasEvent(type);
    }

    return SDL_HasEvents(type, type);
}

/* Override */ SDL_bool SDL_HasEvents(Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_HasEvents);
        return orig::SDL_HasEvents(minType, maxType);
    }

    /* Try to get one event without updating, and return if we got one */
    SDL_Event ev;
    return sdlEventQueue.pop(&ev, 1, minType, maxType, false) ? SDL_TRUE : SDL_FALSE;
}

/* Override */ void SDL_FlushEvent(Uint32 type)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_FlushEvent);
        return orig::SDL_FlushEvent(type);
    }

    return SDL_FlushEvents(type, type);
}

/* Override */ void SDL_FlushEvents(Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_FlushEvents);
        return orig::SDL_FlushEvents(minType, maxType);
    }

    sdlEventQueue.flush(minType, maxType);
}

/* Override */ int SDL_WaitEvent(SDL_Event * event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_WaitEvent);
        return orig::SDL_WaitEvent(event);
    }

    /* From SDL2 code, this function calls SDL_PumpEvents at the beginning */
    NOLOGCALL(SDL_PumpEvents());

    struct timespec mssleep = {0, 1000000};
    if (event) {
        while (! sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true)) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
        }
        return 1;
    }
    else {
        SDL_Event ev;
        while (! sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false)) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
        }
        return 1;
    }
}

/* Override */ int SDL_WaitEventTimeout(SDL_Event * event, int timeout)
{
    debuglogstdio(LCF_SDL | LCF_EVENTS | LCF_TODO, "%s call with timeout %d", __func__, timeout);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_WaitEventTimeout);
        return orig::SDL_WaitEventTimeout(event, timeout);
    }

    /* From SDL2 code, this function calls SDL_PumpEvents at the beginning */
    NOLOGCALL(SDL_PumpEvents());

    int t;
    struct timespec mssleep = {0, 1000000};
    if (event) {
        if (sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true))
            return 1;
        for (t=0; t<timeout; t++) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
            if (sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true))
                break;
        }
        return (t<timeout);
    }
    else {
        SDL_Event ev;
        if (sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false))
            return 1;
        for (t=0; t<timeout; t++) {
            if (sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false))
                break;
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            pushNativeSDLEvents();
        }
        return (t<timeout);
    }
}

/* Override */ int SDL_PushEvent(SDL_Event * event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_PushEvent);
        return orig::SDL_PushEvent(event);
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        SDL1::SDL_Event* ev1 = reinterpret_cast<SDL1::SDL_Event*>(event);

        if (isBannedEvent(ev1))
            return 0;

        int ret = sdlEventQueue.insert(ev1);

        if (ev1->type == SDL1::SDL_QUIT) {
            is_exiting = true;
        }

        return ret; // success
    }

    if (isBannedEvent(event))
        return 0;

    int ret = sdlEventQueue.insert(event);

    if (event->type == SDL_QUIT) {
        is_exiting = true;
    }

    return ret;
}

/* Override */ void SDL_SetEventFilter(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_SetEventFilter);
        return orig::SDL_SetEventFilter(filter, userdata);
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1)
        sdlEventQueue.setFilter(reinterpret_cast<SDL1::SDL_EventFilter>(filter));
    if (SDLver == 2)
        sdlEventQueue.setFilter(filter, userdata);
}

/* Override */ SDL_bool SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_GetEventFilter);
        return orig::SDL_GetEventFilter(filter, userdata);
    }

    int SDLver = get_sdlversion();
    if (SDLver == 1) {
        debuglogstdio(LCF_SDL | LCF_EVENTS | LCF_ERROR, "Not supported yet for SDL1");
        return SDL_FALSE;
        // return reinterpret_cast<SDL_bool>(sdlEventQueue.getFilter());
    }
    if (SDLver == 2) {
        if (sdlEventQueue.getFilter(filter, userdata))
            return SDL_TRUE;
        return SDL_FALSE;
    }

    return SDL_FALSE;
}

/* Override */ void SDL_AddEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_AddEventWatch);
        return orig::SDL_AddEventWatch(filter, userdata);
    }

    sdlEventQueue.addWatch(filter, userdata);
}

/* Override */ void SDL_DelEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_DelEventWatch);
        return orig::SDL_DelEventWatch(filter, userdata);
    }

    sdlEventQueue.delWatch(filter, userdata);
}

/* Override */ void SDL_FilterEvents(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_FilterEvents);
        return orig::SDL_FilterEvents(filter, userdata);
    }

    sdlEventQueue.applyFilter(filter, userdata);
}

/* Override */ Uint8 SDL_EventState(Uint32 type, int state)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_EventState);
        return orig::SDL_EventState(type, state);
    }

    int previousState = sdlEventQueue.isEnabled(type) ? SDL_ENABLE : SDL_DISABLE;
    switch (state) {
        case SDL_ENABLE:
            sdlEventQueue.enable(state);
            return previousState;
        case SDL_DISABLE:
            sdlEventQueue.disable(state);
            return previousState;
        case SDL_QUERY:
            return previousState;
    }
    return state;
}

/* Override */ Uint32 SDL_RegisterEvents(int numevents)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_SDLX(SDL_RegisterEvents);
        return orig::SDL_RegisterEvents(numevents);
    }

    return SDL_USEREVENT;
}

void logEvent(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            debuglogstdio(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving KEYUP/KEYDOWN event with scancode %d and sym %d", (int)event->key.keysym.scancode, event->key.keysym.sym);
            break;

        case SDL_QUIT:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving QUIT event.");
            break;

        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    debuglogstdio(LCF_SDL | LCF_EVENTS, "Window %d gained keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    debuglogstdio(LCF_SDL | LCF_EVENTS, "Window %d lost keyboard focus.", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    debuglogstdio(LCF_SDL | LCF_EVENTS, "Window %d closed.", event->window.windowID);
                    break;
                default:
                    debuglogstdio(LCF_SDL | LCF_EVENTS, "Window event %d", (int)event->window.event);
                    break;
            }
            break;

        case SDL_TEXTEDITING:
            debuglogstdio(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving a keyboard text editing event.");
            break;

        case SDL_TEXTINPUT:
            debuglogstdio(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Receiving a keyboard text input event %s", event->text.text);
            break;
            /*
               case SDL_KEYMAPCHANGED:
               debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a keymap change event.");
               break;
               */
        case SDL_MOUSEMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a mouse move event.");
            break;

        case SDL_MOUSEBUTTONDOWN:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a mouse button press event.");
            break;

        case SDL_MOUSEBUTTONUP:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a mouse button release event.");
            break;

        case SDL_MOUSEWHEEL:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a mouse wheel event.");
            break;

        case SDL_JOYAXISMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick axis motion event.");
            break;

        case SDL_JOYBALLMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick trackball event.");
            break;

        case SDL_JOYHATMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick hat position event.");
            break;

        case SDL_JOYBUTTONDOWN:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick button press event.");
            break;

        case SDL_JOYBUTTONUP:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick button release event.");
            break;

        case SDL_JOYDEVICEADDED:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick connected event.");
            break;

        case SDL_JOYDEVICEREMOVED:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a joystick disconnected event.");
            break;

        case SDL_CONTROLLERAXISMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller axis motion event.");
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller button press event.");
            break;

        case SDL_CONTROLLERBUTTONUP:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller button release event.");
            break;

        case SDL_CONTROLLERDEVICEADDED:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller connected event.");
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller disconnected event.");
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a controller mapping update event.");
            break;

        case SDL_FINGERDOWN:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving an input device touch event.");
            break;

        case SDL_FINGERUP:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving an input device release event.");
            break;

        case SDL_FINGERMOTION:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving an input device drag event.");
            break;

        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
        case SDL_MULTIGESTURE:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a gesture event.");
            break;

        case SDL_CLIPBOARDUPDATE:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a clipboard update event.");
            break;

        case SDL_DROPFILE:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a drag and drop event.");
            break;
            /*
               case SDL_AUDIODEVICEADDED:
               debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a new audio device event.");
               break;

               case SDL_AUDIODEVICEREMOVED:
               debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a audio device removal event.");
               break;
               */
        case SDL_RENDER_TARGETS_RESET:
            //            case SDL_RENDER_DEVICE_RESET:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a render event.");
            break;

        case SDL_USEREVENT:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving a user-specified event.");
            break;

        default:
            debuglogstdio(LCF_SDL | LCF_EVENTS, "Receiving an unknown event: %d", event->type);
            break;
    }
}

}
