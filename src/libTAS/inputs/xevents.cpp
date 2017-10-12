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

#include "xevents.h"
#include "../logging.h"
#include "../hook.h"
//#include "inputs.h"
//#include <X11/XKBlib.h>
//#include <cstring> // memset
//#include "../../shared/AllInputs.h"

namespace libtas {

DEFINE_ORIG_POINTER(XNextEvent);
DEFINE_ORIG_POINTER(XPeekEvent);
DEFINE_ORIG_POINTER(XWindowEvent);
DEFINE_ORIG_POINTER(XCheckWindowEvent);
DEFINE_ORIG_POINTER(XMaskEvent);
DEFINE_ORIG_POINTER(XCheckMaskEvent);
DEFINE_ORIG_POINTER(XCheckTypedEvent);
DEFINE_ORIG_POINTER(XCheckTypedWindowEvent);
DEFINE_ORIG_POINTER(XPending);
DEFINE_ORIG_POINTER(XSendEvent);

/* Return we must prevent an event from being read by the game */
static bool filteredXEvent(XEvent *event)
{
    switch (event->type) {
        case KeyPress:
        case KeyRelease:
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
            return true;
        default:
            return false;
    }
}

static void catchCloseEvent(Display *display, XEvent *event)
{
    if (event->type == ClientMessage) {
        // debuglog(LCF_EVENTS | LCF_WINDOW, "    got a ClientMessage event");
        if ((Atom) event->xclient.data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", False)) {
            debuglog(LCF_EVENTS | LCF_WINDOW, "    caught a window close event");
            is_exiting = true;
        }
    }
}

static void getFirstValidXEvent(Display *display, XEvent *event_return, bool peek)
{
    LINK_NAMESPACE(XNextEvent, nullptr);
    LINK_NAMESPACE(XPeekEvent, nullptr);
    LINK_NAMESPACE(XPending, nullptr);
    orig::XPeekEvent(display, event_return);

    /* Pull events from the queue until the first non-filtered event or
     * until the queue is empty.
     */
    while (filteredXEvent(event_return) && !event_return->xkey.send_event &&
        (orig::XPending(display) > 1)) {
        /* This event is filtered, we pull it and look at the next one */
        orig::XNextEvent(display, event_return);
        orig::XPeekEvent(display, event_return);
    }

    if (filteredXEvent(event_return) && !event_return->xkey.send_event) {
        /* We only pulled filtered events. We must
         * return some event, so returning a dummy event.
         */
        orig::XNextEvent(display, event_return);
        event_return->type = GenericEvent;
        return;
    }

    if (!peek) {
        /* Remove the event from the queue */
        orig::XNextEvent(display, event_return);
    }

    if (filteredXEvent(event_return)) {
        /* Remove the send_event flag so the event looks normal */
        event_return->xkey.send_event = false;
    }

    catchCloseEvent(display, event_return);
}

int XNextEvent(Display *display, XEvent *event_return)
{
    if (GlobalState::isOwnCode()) {
        LINK_NAMESPACE(XNextEvent, nullptr);
        //return orig::XNextEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    getFirstValidXEvent(display, event_return, false);
    return True;
}

int XPeekEvent(Display *display, XEvent *event_return)
{
    if (GlobalState::isOwnCode()) {
        LINK_NAMESPACE(XPeekEvent, nullptr);
        return orig::XPeekEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    getFirstValidXEvent(display, event_return, true);
    return True;
}

int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XWindowEvent);
    return orig::XWindowEvent(display, w, event_mask, event_return);
}

Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    Bool ret = orig::XCheckWindowEvent(display, w, event_mask, event_return);

    /* We must not return a filtered event */
    while ((ret == True) && (filteredXEvent(event_return) && !event_return->xkey.send_event)) {
        ret = orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    catchCloseEvent(display, event_return);
    return ret;
}

int XMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XMaskEvent);
    return orig::XMaskEvent(display, event_mask, event_return);
}

Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    Bool ret = orig::XCheckMaskEvent(display, event_mask, event_return);

    /* We must not return a filtered event */
    while ((ret == True) && (filteredXEvent(event_return) && !event_return->xkey.send_event)) {
        ret = orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    catchCloseEvent(display, event_return);
    return ret;
}

Bool XCheckTypedEvent(Display *display, int event_type, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckTypedEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckTypedEvent(display, event_type, event_return);
    }

    Bool ret = orig::XCheckTypedEvent(display, event_type, event_return);

    /* We must not return a filtered event */
    while ((ret == True) && (filteredXEvent(event_return) && !event_return->xkey.send_event)) {
        ret = orig::XCheckTypedEvent(display, event_type, event_return);
    }

    catchCloseEvent(display, event_return);
    return ret;
}

Bool XCheckTypedWindowEvent(Display *display, Window w, int event_type, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckTypedWindowEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckTypedWindowEvent(display, w, event_type, event_return);
    }

    Bool ret = orig::XCheckTypedWindowEvent(display, w, event_type, event_return);

    /* We must not return a filtered event */
    while ((ret == True) && (filteredXEvent(event_return) && !event_return->xkey.send_event)) {
        ret = orig::XCheckTypedWindowEvent(display, w, event_type, event_return);
    }

    catchCloseEvent(display, event_return);
    return ret;
}

int XPending(Display *display)
{
    LINK_NAMESPACE(XPending, nullptr);
    if (GlobalState::isOwnCode()) {
        return orig::XPending(display);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    int ret = orig::XPending(display);
    debuglog(LCF_EVENTS, "    returns ", ret);
    return ret;
}

Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
    LINK_NAMESPACE(XSendEvent, nullptr);
    if (GlobalState::isOwnCode()) {
        return orig::XSendEvent(display, w, propagate, event_mask, event_send);
    }
    return orig::XSendEvent(display, w, propagate, event_mask, event_send);
}


}
