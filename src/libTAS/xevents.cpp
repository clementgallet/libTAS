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

#include "xevents.h"
#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(XCheckIfEvent);
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

/* Function to indicate if an event is filtered */
static Bool isEventFiltered (Display *display, XEvent *event, XPointer arg) {
    if (event->xkey.send_event) {
        return False;
    }
    switch (event->type) {
        case KeyPress:
        case KeyRelease:
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case FocusIn:
        case FocusOut:
            return True;
        default:
            return False;
    }
}

/* Removes all filtered events from the event queue */
static void filterEventQueue(Display *display)
{
    LINK_NAMESPACE_GLOBAL(XCheckIfEvent);
    XEvent event;
    XPointer arg = nullptr;
    while (True == XCheckIfEvent(display, &event, isEventFiltered, arg)) {}
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

int XNextEvent(Display *display, XEvent *event_return)
{
    LINK_NAMESPACE_GLOBAL(XNextEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XNextEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    filterEventQueue(display);
    int ret = orig::XNextEvent(display, event_return);

    catchCloseEvent(display, event_return);

    return ret;
}

int XPeekEvent(Display *display, XEvent *event_return)
{
    LINK_NAMESPACE_GLOBAL(XPeekEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XPeekEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    filterEventQueue(display);
    int ret = orig::XPeekEvent(display, event_return);

    catchCloseEvent(display, event_return);
    return ret;
}

int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XWindowEvent);
    filterEventQueue(display);
    int ret = orig::XWindowEvent(display, w, event_mask, event_return);
    catchCloseEvent(display, event_return);
    return ret;
}

Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
    filterEventQueue(display);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    Bool ret = orig::XCheckWindowEvent(display, w, event_mask, event_return);
    catchCloseEvent(display, event_return);
    return ret;
}

int XMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XMaskEvent);
    filterEventQueue(display);
    int ret = orig::XMaskEvent(display, event_mask, event_return);
    catchCloseEvent(display, event_return);
    return ret;
}

Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }
    filterEventQueue(display);

    Bool ret = orig::XCheckMaskEvent(display, event_mask, event_return);

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
    filterEventQueue(display);

    Bool ret = orig::XCheckTypedEvent(display, event_type, event_return);

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
    filterEventQueue(display);

    Bool ret = orig::XCheckTypedWindowEvent(display, w, event_type, event_return);

    catchCloseEvent(display, event_return);
    return ret;
}


int XPending(Display *display)
{
    LINK_NAMESPACE_GLOBAL(XPending);
    if (GlobalState::isOwnCode()) {
        return orig::XPending(display);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    filterEventQueue(display);

    int ret = orig::XPending(display);
    debuglog(LCF_EVENTS, "    returns ", ret);
    return ret;
}

Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
    LINK_NAMESPACE_GLOBAL(XSendEvent);
    if (GlobalState::isOwnCode()) {
        return orig::XSendEvent(display, w, propagate, event_mask, event_send);
    }
    DEBUGLOGCALL(LCF_EVENTS);
    return orig::XSendEvent(display, w, propagate, event_mask, event_send);
}

}
