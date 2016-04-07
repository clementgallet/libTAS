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

#include "events.h"
#include "logging.h"
#include "hook.h"
#include "inputs/inputs.h"
#include "windows.h" // for gameWindow variable
#include <stdarg.h>

/* Pointers to original functions */
void (*SDL_PumpEvents_real)(void);
int (*SDL_PeepEvents_real)(SDL_Event*, int, SDL_eventaction, Uint32, Uint32);
int (*SDL1_PeepEvents_real)(SDL1::SDL_Event*, int, SDL_eventaction, Uint32);

/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, ...)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_FRAME);

    /* We need to use a function signature with variable arguments,
     * because SDL 1.2 and SDL 2 provide a different function with the same name.
     * SDL 1.2 is int SDL_PeepEvents(SDL1_Event *events, int numevents, SDL_eventaction action, Uint32 mask);
     * SDL 2   is int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
     */

    Uint32 mask;
    Uint32 minType, maxType;

    va_list argp;
    va_start(argp, action);
    if (SDLver == 1) {
        mask = va_arg(argp, Uint32);
    }
    else if (SDLver == 2) {
        minType = va_arg(argp, Uint32);
        maxType = va_arg(argp, Uint32);
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Calling SDL_PeepEvents with no known SDL version!");
    }
    va_end(argp);
    
    switch (action) {
        case SDL_ADDEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to add ", numevents, " events");
            if (SDLver == 1)
                return SDL1_PeepEvents_real((SDL1::SDL_Event*) events, numevents, action, mask);
            if (SDLver == 2)
                return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
            break;
        case SDL_PEEKEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to peek at ", numevents, " events");
            if (SDLver == 1)
                return getSDL1Events((SDL1::SDL_Event*) events, numevents, 0, mask);
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 0, minType, maxType);
            break;
        case SDL_GETEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to get ", numevents, " events");
            if (SDLver == 1)
                return getSDL1Events((SDL1::SDL_Event*) events, numevents, 1, mask);
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 1, minType, maxType);
            break;
    }

    return 0;
}

/* Override */ int SDL_PollEvent(SDL_Event *event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_FRAME);

    /* 
     * SDL_PollEvent is supposed to call SDL_PumpEvents,
     * so we are doing it ourselves
     */
    SDL_PumpEvents_real();

    if (event) {
        /* Fetch one event with update using our helper function */
        if (SDLver == 1)
            return getSDL1Events((SDL1::SDL_Event*)event, 1, 1, SDL1::SDL_ALLEVENTS);
        if (SDLver == 2)
            return getSDL2Events(event, 1, 1, SDL_FIRSTEVENT, SDL_LASTEVENT);
    } else {
        /*
         * In the case the event pointer is NULL, SDL doc says to
         * return 1 if there is an event in the queue and 0 of not,
         * without updating the queue
         */
        if (SDLver == 1) {
            SDL1::SDL_Event ev1;
            return getSDL1Events(&ev1, 1, 0, SDL1::SDL_ALLEVENTS);
        }
        if (SDLver == 2) {
            SDL_Event ev;
            return getSDL2Events(&ev, 1, 0, SDL_FIRSTEVENT, SDL_LASTEVENT);
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
    return getSDL2Events(&ev, 1, 0, minType, maxType) ? SDL_TRUE : SDL_FALSE;
}

/* Override */ void SDL_FlushEvent(Uint32 type)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);
    return SDL_FlushEvents(type, type);
}

/* Override */ void SDL_FlushEvents(Uint32 minType, Uint32 maxType)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    /* Get events from the queue until we get nothing */
    const int packet = 16;
    SDL_Event evs[packet];
    while (getSDL2Events(evs, packet, 1, minType, maxType)) {}

}

/* Override */ int SDL_WaitEvent(SDL_Event * event)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS);

    SDL_PumpEvents_real();

    struct timespec mssleep = {0, 1000000};
    if (event) {
        while (! getSDL2Events(event, 1, 1, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            nanosleep_real(&mssleep, NULL); // Wait 1 ms before trying again
            SDL_PumpEvents_real();
        }
        return 1;
    }
    else {
        SDL_Event ev;
        while (! getSDL2Events(&ev, 1, 0, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
            nanosleep_real(&mssleep, NULL); // Wait 1 ms before trying again
            SDL_PumpEvents_real();
        }
        return 1;
    }
}

/* Override */ int SDL_WaitEventTimeout(SDL_Event * event, int timeout)
{
    debuglog(LCF_SDL | LCF_EVENTS | LCF_TIMEFUNC, __func__, " call with timeout ", timeout);

    int t;
    struct timespec mssleep = {0, 1000000};
    if (event) {
        for (t=0; t<timeout; t++) {
            SDL_PumpEvents_real();
            if (getSDL2Events(event, 1, 1, SDL_FIRSTEVENT, SDL_LASTEVENT))
                break;
            nanosleep_real(&mssleep, NULL); // Wait 1 ms before trying again
        }
        return (t<timeout);
    }
    else {
        for (t=0; t<timeout; t++) {
            SDL_PumpEvents_real();
            if (getSDL2Events(event, 1, 0, SDL_FIRSTEVENT, SDL_LASTEVENT))
                break;
            nanosleep_real(&mssleep, NULL); // Wait 1 ms before trying again
        }
        return (t<timeout);
    }
}

void SDL_SetEventFilter(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
}

SDL_bool SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
    return SDL_FALSE;
}

void SDL_AddEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
}

void SDL_DelEventWatch(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
}

void SDL_FilterEvents(SDL_EventFilter filter, void *userdata)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
}

Uint8 SDL_EventState(Uint32 type, int state)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
    return /*SDL_ENABLE*/ 1;
}

Uint32 SDL_RegisterEvents(int numevents)
{
    DEBUGLOGCALL(LCF_SDL | LCF_EVENTS | LCF_TODO);
    return SDL_USEREVENT;
}

int getSDL2Events(SDL_Event *events, int numevents, int update, Uint32 minType, Uint32 maxType)
{

    /* Total number of events pulled from SDL_PeepEvents call */
    int peepnb = 0;

    if (update)
        peepnb = SDL_PeepEvents_real(events, numevents, SDL_GETEVENT, minType, maxType);
    else
        peepnb = SDL_PeepEvents_real(events, numevents, SDL_PEEKEVENT, minType, maxType);

    /* 
     * Among all the events we pulled, we have to filter some
     * (e.g. input type).
     */

    for (int peepi = 0; peepi < peepnb; peepi++) {
        if (filterSDL2Event(&(events[peepi]))) {
            /* 
             * We have to filter this event.
             * For now, let's just attribute an unused type.
             * We will remove all filtered events later.
             */
            events[peepi].type = SDL_FIRSTEVENT;
        }
    }

    /* Now we remove all filtered events from the array
     *
     * | e0 | e1 | e2 | -- | -- | e3 | e4 |
     *                  pd        ps
     *                   _
     *                  /\________/
     */

    int ps = 0, pd = 0;
    while (ps < peepnb) {
        if (events[pd].type != SDL_FIRSTEVENT) {
            logEvent(&events[pd]);
            pd++;
            if (pd > ps)
                ps++;
        }
        else if (events[ps].type == SDL_FIRSTEVENT) {
            ps++;
        }
        else {
            /* We are in a position to copy event ps to event pd */
            events[pd] = events[ps];
            events[ps].type = SDL_FIRSTEVENT;
            logEvent(&events[pd]);
            ps++;
            pd++;
        }
    }

    /* We update the total number of saved events */
    peepnb = pd;

    /* 
     * TODO: Actually because of filtered events, we should make
     * multiple passes to PeepEvents if we did not get the right number,
     * but it gets complicated.
     */

    if (peepnb == numevents) {
        /* We got the right number of events from the real event queue */
        return peepnb;
    }

    /* 
     * We did not get enough events with the event queue only.
     * Now we return our custom events.
     */

    /* Generate controllers plugged in.
     * Some games use this to detect controllers instead of using SDL_NumJoysticks
     */
    if ((SDL_CONTROLLERDEVICEADDED >= minType) && (SDL_CONTROLLERDEVICEADDED <= maxType)) {
        peepnb += generateControllerAdded(&events[peepnb], numevents - peepnb, update);
    }
    if (peepnb == numevents) return peepnb;

    /*
     * Important: You must call the following two functions in that order!
     * This is because the first one updates old_ai by removing elements
     * to match ai, and the second one updates old_ai by adding elements
     * to match ai. So if you call the second one before the first,
     * the keyboard array in old_ai can overflow.
     */

    /* Getting KeyUp events */
    if ((SDL_KEYUP >= minType) && (SDL_KEYUP <= maxType))
        peepnb += generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

    if (peepnb == numevents) return peepnb;

    if ((SDL_KEYDOWN >= minType) && (SDL_KEYDOWN <= maxType)) {

        /* We must add this failsafe concerning the above comment.
         * If we must update SDL_KEYDOWN but did not update SDL_KEYUP,
         * this is bad. So we must update SDL_KEYUP events
         * and discard the result.
         */

        if (update && ((SDL_KEYUP < minType) || (SDL_KEYUP > maxType)))
            /* Update KEYUP events */
            generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

        peepnb += generateKeyDownEvent(&events[peepnb], gameWindow, numevents - peepnb, update);
    }

    if (peepnb == numevents) return peepnb;


    if ((SDL_CONTROLLERAXISMOTION >= minType) && (SDL_CONTROLLERAXISMOTION <= maxType))
        /* TODO: Split the function into functions for each event type,
         * or pass the event filters to the function
         */
        peepnb += generateControllerEvent(&events[peepnb], numevents - peepnb, update);

    return peepnb;
}


int getSDL1Events(SDL1::SDL_Event *events, int numevents, int update, Uint32 mask)
{

    /* Total number of events pulled from SDL_PeepEvents call */
    int peepnb = 0;

    if (update)
        peepnb = SDL1_PeepEvents_real(events, numevents, SDL_GETEVENT, mask);
    else
        peepnb = SDL1_PeepEvents_real(events, numevents, SDL_PEEKEVENT, mask);

    /* 
     * Among all the events we pulled, we have to filter some
     * (e.g. input type).
     */

    for (int peepi = 0; peepi < peepnb; peepi++) {
        if (filterSDL1Event(&(events[peepi]))) {
            /* 
             * We have to filter this event.
             * For now, let's just attribute an unused type.
             * We will remove all filtered events later.
             */
            events[peepi].type = SDL1::SDL_NOEVENT;
        }
    }

    /* Now we remove all filtered events from the array
     *
     * | e0 | e1 | e2 | -- | -- | e3 | e4 |
     *                  pd        ps
     *                   _
     *                  /\________/
     */

    int ps = 0, pd = 0;
    while (ps < peepnb) {
        if (events[pd].type != SDL1::SDL_NOEVENT) {
            pd++;
            if (pd > ps)
                ps++;
        }
        else if (events[ps].type == SDL1::SDL_NOEVENT) {
            ps++;
        }
        else {
            /* We are in a position to copy event ps to event pd */
            events[pd] = events[ps];
            events[ps].type = SDL1::SDL_NOEVENT;
            ps++;
            pd++;
        }
    }

    /* We update the total number of saved events */
    peepnb = pd;

    /* 
     * TODO: Actually because of filtered events, we should make
     * multiple passes to PeepEvents if we did not get the right number,
     * but it gets complicated.
     */

    if (peepnb == numevents) {
        /* We got the right number of events from the real event queue */
        return peepnb;
    }

    /* 
     * We did not get enough events with the event queue only.
     * Now we return our custom events.
     */

    /*
     * Important: You must call the following two functions in that order!
     * This is because the first one updates old_ai by removing elements
     * to match ai, and the second one updates old_ai by adding elements
     * to match ai. So if you call the second one before the first,
     * the keyboard array in old_ai can overflow.
     */

    /* Getting KeyUp events */
    if (mask & SDL1::SDL_KEYUPMASK)
        peepnb += generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

    if (peepnb == numevents) return peepnb;

    if (mask & SDL1::SDL_KEYDOWNMASK) {

        /* We must add this failsafe concerning the above comment.
         * If we must update SDL_KEYDOWN but did not update SDL_KEYUP,
         * this is bad. So we must update SDL_KEYUP events
         * and discard the result.
         */

        if (update && (! (mask & SDL1::SDL_KEYUPMASK)))
            /* Update KEYUP events */
            generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

        peepnb += generateKeyDownEvent(&events[peepnb], gameWindow, numevents - peepnb, update);
    }

    return peepnb;
}

int filterSDL2Event(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
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
            return 1;
        default:
            return 0;
    }
}

int filterSDL1Event(SDL1::SDL_Event *event)
{
    switch(event->type) {
        case SDL1::SDL_KEYDOWN:
        case SDL1::SDL_KEYUP:
            return 1;
        default:
            return 0;
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
    if (SDLver == 1) {
        link_function((void**)&SDL1_PeepEvents_real, "SDL_PeepEvents", "libSDL-1.2");
    }
    if (SDLver == 2) {
        LINK_SUFFIX_SDL2(SDL_PeepEvents);
    }
    LINK_SUFFIX_SDLX(SDL_PumpEvents);
}

