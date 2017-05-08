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

#include "sdlevents.h"
#include "logging.h"
#include "hook.h"
#include "sdlwindows.h" // for gameWindow variable
#include "EventQueue.h"
#include "sleep.h"
#include "ThreadState.h"

/* Pointers to original functions */
namespace orig {
    static void (*SDL_PumpEvents)(void);
    static int (*SDL_PeepEvents)(void *events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
}

void pushNativeEvents(void)
{
    /* SDL_PumpEvents may call SDL_GetTicks() a lot, and we don't want to
     * advance the timer because of that, so we make it untrack
     */
    ThreadOwnCode toc;

    orig::SDL_PumpEvents();

    /* We use SDL_PeepEvents() for gathering events from the SDL queue,
     * as it is the native function of getting events.
     * i.e. all other functions call this function internally.
     */
    if (SDLver == 1) {
        SDL1::SDL_Event ev;
        while (orig::SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL1::SDL_ALLEVENTS, 0)) {
            if (! filterSDL1Event(&ev))
                sdlEventQueue.insert(&ev);
        }
    }

    if (SDLver == 2) {
        SDL_Event ev;
        while (orig::SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            if (! filterSDL2Event(&ev))
                sdlEventQueue.insert(&ev);
        }
    }
}


void pushQuitEvent(void)
{
    if (SDLver == 1) {
        SDL1::SDL_Event ev;
        ev.type = SDL1::SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }

    if (SDLver == 2) {
        SDL_Event ev;
        ev.type = SDL_QUIT;
        sdlEventQueue.insert(&ev);
    }
}

/* Override */ void SDL_PumpEvents(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_FRAME);
    return;
}

/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_FRAME);

    /* We need to use a function signature with variable arguments,
     * because SDL 1.2 and SDL 2 provide a different function with the same name.
     * SDL 1.2 is int SDL_PeepEvents(SDL1_Event *events, int numevents, SDL_eventaction action, Uint32 mask);
     * SDL 2   is int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
     */

    Uint32 mask;
    SDL1::SDL_Event* events1;

    if (SDLver == 1) {
        mask = minType;
        events1 = reinterpret_cast<SDL1::SDL_Event*>(events);
    }

    switch (action) {
        case SDL_ADDEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to add ", numevents, " events");
            for (int i=0; i<numevents; i++) {
                if (SDLver == 1)
                    if (! filterSDL1Event(&events1[i]))
                        sdlEventQueue.insert(&events1[i]);
                if (SDLver == 2)
                    if (! filterSDL2Event(&events[i]))
                        sdlEventQueue.insert(&events[i]);
            }
            break;
        case SDL_PEEKEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to peek at ", numevents, " events");
            if (SDLver == 1)
                return sdlEventQueue.pop(events1, numevents, mask, false);
            if (SDLver == 2)
                return sdlEventQueue.pop(events, numevents, minType, maxType, false);
            break;
        case SDL_GETEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to get ", numevents, " events");
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
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_FRAME);

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
    return SDL_HasEvents(type, type);
}

/* Override */ SDL_bool SDL_HasEvents(Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    /* Try to get one event without updating, and return if we got one */
    SDL_Event ev;
    return sdlEventQueue.pop(&ev, 1, minType, maxType, false) ? SDL_TRUE : SDL_FALSE;
}

/* Override */ void SDL_FlushEvent(Uint32 type)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    return SDL_FlushEvents(type, type);
}

/* Override */ void SDL_FlushEvents(Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    sdlEventQueue.flush(minType, maxType);
}

/* Override */ int SDL_WaitEvent(SDL_Event * event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    struct timespec mssleep = {0, 1000000};
    if (event) {
        while (! sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true)) {
            orig::nanosleep(&mssleep, NULL); // Wait 1 ms before trying again
            pushNativeEvents();
        }
        return 1;
    }
    else {
        SDL_Event ev;
        while (! sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false)) {
            orig::nanosleep(&mssleep, NULL); // Wait 1 ms before trying again
            pushNativeEvents();
        }
        return 1;
    }
}

/* Override */ int SDL_WaitEventTimeout(SDL_Event * event, int timeout)
{
    debuglog(LCF_SDL | LCF_EVENTS | LCF_TIMEFUNC | LCF_TODO, __func__, " call with timeout ", timeout);

    int t;
    struct timespec mssleep = {0, 1000000};
    if (event) {
        for (t=0; t<timeout; t++) {
            if (sdlEventQueue.pop(event, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, true))
                break;
            orig::nanosleep(&mssleep, NULL); // Wait 1 ms before trying again
            pushNativeEvents();
        }
        return (t<timeout);
    }
    else {
        SDL_Event ev;
        for (t=0; t<timeout; t++) {
            if (sdlEventQueue.pop(&ev, 1, SDL_FIRSTEVENT, SDL_LASTEVENT, false))
                break;
            orig::nanosleep(&mssleep, NULL); // Wait 1 ms before trying again
            pushNativeEvents();
        }
        return (t<timeout);
    }
}

/* Override */ int SDL_PushEvent(SDL_Event * event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (SDLver == 1) {
        SDL1::SDL_Event* ev1 = reinterpret_cast<SDL1::SDL_Event*>(event);
        sdlEventQueue.insert(ev1);
        /* TODO: Support event queue full */
        return 0; // success
    }

    if (filterSDL2Event(event))
        return 0;

    sdlEventQueue.insert(event);
    /* TODO: Support event queue full */
    return 1; // success
}

/* Override */ void SDL_SetEventFilter(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    if (SDLver == 1)
        sdlEventQueue.setFilter(reinterpret_cast<SDL1::SDL_EventFilter>(filter));
    if (SDLver == 2)
        sdlEventQueue.setFilter(filter, userdata);
}

/* Override */ void* SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    if (SDLver == 1)
        return reinterpret_cast<void*>(sdlEventQueue.getFilter());
    if (SDLver == 2) {
        if (sdlEventQueue.getFilter(filter, userdata))
            return reinterpret_cast<void*>(SDL_TRUE);
        return reinterpret_cast<void*>(SDL_FALSE);
    }

    return NULL;
}

/* Override */ void SDL_AddEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    sdlEventQueue.addWatch(filter, userdata);
}

/* Override */ void SDL_DelEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    sdlEventQueue.delWatch(filter, userdata);
}

/* Override */ void SDL_FilterEvents(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    sdlEventQueue.applyFilter(filter, userdata);
}

/* Override */ Uint8 SDL_EventState(Uint32 type, int state)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
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
    return SDL_USEREVENT;
}

bool filterSDL2Event(SDL_Event *event)
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
        default:
            return false;
    }
}

bool filterSDL1Event(SDL1::SDL_Event *event)
{
    switch(event->type) {
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

void logEvent(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving KEYUP/KEYDOWN event.");
            break;

        case SDL_QUIT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving QUIT event.");
            break;

        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window ", event->window.windowID, " gained keyboard focus.");
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window ", event->window.windowID, " lost keyboard focus.");
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    debuglog(LCF_SDL | LCF_EVENTS, "Window ", event->window.windowID, " closed.");
                    break;
                default:
                    break;
            }
            break;

        case SDL_SYSWMEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a system specific event.");
            switch (event->syswm.msg->subsystem) {
                case SDL_SYSWM_UNKNOWN:
                    debuglog(LCF_SDL | LCF_EVENTS, "Unknown subsystem.");
                    break;
                case SDL_SYSWM_WINDOWS:
                    debuglog(LCF_SDL | LCF_EVENTS, "Windows subsystem.");
                    break;
                case SDL_SYSWM_X11:
                    debuglog(LCF_SDL | LCF_EVENTS, "X subsystem.");
                    debuglog(LCF_SDL | LCF_EVENTS, "Getting an X event of type ", event->syswm.msg->msg.x11.event.type);
                    break;
                case SDL_SYSWM_DIRECTFB:
                    debuglog(LCF_SDL | LCF_EVENTS, "DirectFB subsystem.");
                    break;
                case SDL_SYSWM_COCOA:
                    debuglog(LCF_SDL | LCF_EVENTS, "OSX subsystem.");
                    break;
                case SDL_SYSWM_UIKIT:
                    debuglog(LCF_SDL | LCF_EVENTS, "iOS subsystem.");
                    break;
                default:
                    debuglog(LCF_SDL | LCF_EVENTS, "Another subsystem.");
                    break;
            }
            break;

        case SDL_TEXTEDITING:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keyboard text editing event.");
            break;

        case SDL_TEXTINPUT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keyboard text input event.");
            break;
            /*
               case SDL_KEYMAPCHANGED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a keymap change event.");
               break;
               */
        case SDL_MOUSEMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse move event.");
            break;

        case SDL_MOUSEBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse button press event.");
            break;

        case SDL_MOUSEBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse button release event.");
            break;

        case SDL_MOUSEWHEEL:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a mouse wheel event.");
            break;

        case SDL_JOYAXISMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick axis motion event.");
            break;

        case SDL_JOYBALLMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick trackball event.");
            break;

        case SDL_JOYHATMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick hat position event.");
            break;

        case SDL_JOYBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick button press event.");
            break;

        case SDL_JOYBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick button release event.");
            break;

        case SDL_JOYDEVICEADDED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick connected event.");
            break;

        case SDL_JOYDEVICEREMOVED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a joystick disconnected event.");
            break;

        case SDL_CONTROLLERAXISMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller axis motion event.");
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller button press event.");
            break;

        case SDL_CONTROLLERBUTTONUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller button release event.");
            break;

        case SDL_CONTROLLERDEVICEADDED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller connected event.");
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller disconnected event.");
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a controller mapping update event.");
            break;

        case SDL_FINGERDOWN:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device touch event.");
            break;

        case SDL_FINGERUP:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device release event.");
            break;

        case SDL_FINGERMOTION:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an input device drag event.");
            break;

        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
        case SDL_MULTIGESTURE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a gesture event.");
            break;

        case SDL_CLIPBOARDUPDATE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a clipboard update event.");
            break;

        case SDL_DROPFILE:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a drag and drop event.");
            break;
            /*
               case SDL_AUDIODEVICEADDED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a new audio device event.");
               break;

               case SDL_AUDIODEVICEREMOVED:
               debuglog(LCF_SDL | LCF_EVENTS, "Receiving a audio device removal event.");
               break;
               */
        case SDL_RENDER_TARGETS_RESET:
            //            case SDL_RENDER_DEVICE_RESET:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a render event.");
            break;

        case SDL_USEREVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving a user-specified event.");
            break;

        default:
            debuglog(LCF_SDL | LCF_EVENTS, "Receiving an unknown event: ", event->type, ".");
            break;
    }
}

void link_sdlevents(void)
{
    LINK_NAMESPACE_SDLX(SDL_PumpEvents);
    LINK_NAMESPACE_SDLX(SDL_PeepEvents);
}
