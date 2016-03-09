#include "events.h"
#include "logging.h"
#include "hook.h"
#include "inputs.h"
#include "libTAS.h" // for gameWindow variable
#include <stdarg.h>


/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, ...)
{
    debuglog(LCF_SDL | LCF_EVENTS, __func__, " call.");

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
                return SDL1_PeepEvents_real((SDL1_Event*) events, numevents, action, mask);
            if (SDLver == 2)
                return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
            break;
        case SDL_PEEKEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to peek at ", numevents, " events");
            if (SDLver == 1)
                return getSDL1Events((SDL1_Event*) events, numevents, 0, mask);
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 0, minType, maxType);
            break;
        case SDL_GETEVENT:
            debuglog(LCF_SDL | LCF_EVENTS, "The game wants to get ", numevents, " events");
            if (SDLver == 1)
                return getSDL1Events((SDL1_Event*) events, numevents, 1, mask);
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 1, minType, maxType);
            break;
    }

    return 0;
}

/* Override */ int SDL_PollEvent(SDL_Event *event)
{
    debuglog(LCF_SDL | LCF_EVENTS, __func__, " call.");

    /* 
     * SDL_PollEvent is supposed to call SDL_PumpEvents,
     * so we are doing it ourselves
     */
    SDL_PumpEvents_real();

    if (event) {
        /* Fetch one event with update using our helper function */
        if (SDLver == 1)
            return getSDL1Events((SDL1_Event*)event, 1, 1, SDL1_ALLEVENTS);
        if (SDLver == 2)
            return getSDL2Events(event, 1, 1, SDL_FIRSTEVENT, SDL_LASTEVENT);
    } else {
        /*
         * In the case the event pointer is NULL, SDL doc says to
         * return 1 if there is an event in the queue and 0 of not,
         * without updating the queue
         */
        if (SDLver == 1) {
            SDL1_Event ev1;
            return getSDL1Events(&ev1, 1, 0, SDL1_ALLEVENTS);
        }
        if (SDLver == 2) {
            SDL_Event ev;
            return getSDL2Events(&ev, 1, 0, SDL_FIRSTEVENT, SDL_LASTEVENT);
        }
    }
    return -1;
}

/* 
 * This helper function will return a number of events from the generated event queue.
 * This event queue consists on the real SDL event queue with our own filter
 * (e.g. removing real input events)
 * and generated SDL events containing mostly inputs passed from linTAS.
 *
 * Because SDL has multiple ways of accessing the event queue, we made this function
 * with two parameter indicating the number of events we want and if we need 
 * to update the queue by removing the returned event, or keep it in the queue.
 * 
 * The function returns the number of events returned.
 */
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

/* 
 * Same as the getSDL2Events, except that we are dealing with SDL 1.2 events.
 * These events have a different structure, and the filtering is also different.
 * Otherwise, the function acheive the same goal.
 */

int getSDL1Events(SDL1_Event *events, int numevents, int update, Uint32 mask)
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
            events[peepi].type = SDL1_NOEVENT;
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
        if (events[pd].type != SDL1_NOEVENT) {
            pd++;
            if (pd > ps)
                ps++;
        }
        else if (events[ps].type == SDL1_NOEVENT) {
            ps++;
        }
        else {
            /* We are in a position to copy event ps to event pd */
            events[pd] = events[ps];
            events[ps].type = SDL1_NOEVENT;
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
    if (mask & SDL1_KEYUPMASK)
        peepnb += generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

    if (peepnb == numevents) return peepnb;

    if (mask & SDL1_KEYDOWNMASK) {

        /* We must add this failsafe concerning the above comment.
         * If we must update SDL_KEYDOWN but did not update SDL_KEYUP,
         * this is bad. So we must update SDL_KEYUP events
         * and discard the result.
         */

        if (update && (! (mask & SDL1_KEYUPMASK)))
            /* Update KEYUP events */
            generateKeyUpEvent(&events[peepnb], gameWindow, numevents - peepnb, update);

        peepnb += generateKeyDownEvent(&events[peepnb], gameWindow, numevents - peepnb, update);
    }

    return peepnb;
}

/* Return if the SDL event must be passed to the game or be filtered */
int filterSDL2Event(SDL_Event *event)
{
    switch(event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return 1;
        default:
            return 0;
    }
}

/* Return if the SDL event must be passed to the game or be filtered */
int filterSDL1Event(SDL1_Event *event)
{
    switch(event->type) {
        case SDL1_KEYDOWN:
        case SDL1_KEYUP:
            return 1;
        default:
            return 0;
    }
}

/* Print which event type is it */
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

