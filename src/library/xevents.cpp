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
#include "XlibEventQueue.h"

#ifdef LIBTAS_HAS_XINPUT
#include <X11/extensions/XInput2.h>
#endif

namespace libtas {

DEFINE_ORIG_POINTER(XCheckIfEvent);
DEFINE_ORIG_POINTER(XIfEvent);
DEFINE_ORIG_POINTER(XNextEvent);
DEFINE_ORIG_POINTER(XPeekEvent);
DEFINE_ORIG_POINTER(XWindowEvent);
DEFINE_ORIG_POINTER(XCheckWindowEvent);
DEFINE_ORIG_POINTER(XMaskEvent);
DEFINE_ORIG_POINTER(XCheckMaskEvent);
DEFINE_ORIG_POINTER(XCheckTypedEvent);
DEFINE_ORIG_POINTER(XCheckTypedWindowEvent);
DEFINE_ORIG_POINTER(XEventsQueued);
DEFINE_ORIG_POINTER(XPending);
DEFINE_ORIG_POINTER(XSendEvent);
DEFINE_ORIG_POINTER(XSync);
DEFINE_ORIG_POINTER(XGetEventData);
DEFINE_ORIG_POINTER(XFreeEventData);

/* Function to indicate if an event is filtered */
static Bool isEventFiltered (XEvent *event) {
    switch (event->type) {
        case KeyPress:
        case KeyRelease:
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case GenericEvent:
        case FocusIn:
        case FocusOut:
        case Expose:
        case EnterNotify:
        case LeaveNotify:
        case PropertyNotify:
        case ReparentNotify:
            return True;
        default:
            return False;
    }
}

void pushNativeXlibEvents(void)
{
    LINK_NAMESPACE_GLOBAL(XPending);
    LINK_NAMESPACE_GLOBAL(XNextEvent);
    LINK_NAMESPACE_GLOBAL(XSync);

    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (gameDisplays[i]) {
            XSync(gameDisplays[i], False);
            while (orig::XPending(gameDisplays[i]) > 0) {
                XEvent event;
                orig::XNextEvent(gameDisplays[i], &event);

                /* Catch the close event */
                if (event.type == ClientMessage) {
                    static Atom dwAtom = XInternAtom(gameDisplays[i], "WM_DELETE_WINDOW", False);
                    if ((Atom) event.xclient.data.l[0] == dwAtom) {
                        debuglog(LCF_EVENTS | LCF_WINDOW, "    caught a window close event");
                        is_exiting = true;
                    }
                }

                if (!isEventFiltered(&event)) {
                    xlibEventQueue.insert(&event);
                }
            }
        }
    }
}

int XNextEvent(Display *display, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XNextEvent);
        return orig::XNextEvent(display, event_return);
    }

    bool isEvent = false;
    for (int r=0; r<1000; r++) {
        isEvent = xlibEventQueue.pop(event_return, true);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents();
    }
    if (!isEvent) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;

    // debuglog(LCF_KEYBOARD, "    got event ", event_return->type);
    //
    // if (event_return && (event_return->type == KeyPress)) {
    //     event_return->xkey.time = 0;
    //     event_return->xkey.x = 0;
    //     event_return->xkey.y = 0;
    //     event_return->xkey.x_root = 0;
    //     event_return->xkey.y_root = 0;
    //     event_return->xkey.send_event = 0;
    //     debuglog(LCF_KEYBOARD, "    xkeyevent send_event", event_return->xkey.send_event);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent display", event_return->xkey.display);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent type", event_return->xkey.type);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent state", event_return->xkey.state);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent window", event_return->xkey.window);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent time", event_return->xkey.time);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent same_screen", event_return->xkey.same_screen);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent keycode", event_return->xkey.keycode);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent root", event_return->xkey.root);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent subwindow", event_return->xkey.subwindow);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent x", event_return->xkey.x);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent y", event_return->xkey.y);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent x_root", event_return->xkey.x_root);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent y_root", event_return->xkey.y_root);
    //     debuglog(LCF_KEYBOARD, "    xkeyevent keycode", event_return->xkey.keycode);
    // }
}

int XPeekEvent(Display *display, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XPeekEvent);
        return orig::XPeekEvent(display, event_return);
    }

    bool isEvent = false;
    for (int r=0; r<1000; r++) {
        isEvent = xlibEventQueue.pop(event_return, false);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents();
    }
    if (!isEvent) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XWindowEvent);
        return orig::XWindowEvent(display, w, event_mask, event_return);
    }

    bool isEvent = false;
    for (int r=0; r<1000; r++) {
        isEvent = xlibEventQueue.pop(event_return, w, event_mask);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents();
    }
    if (!isEvent) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    bool isEvent = xlibEventQueue.pop(event_return, w, event_mask);
    return isEvent?True:False;
}

int XMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XMaskEvent);
        return orig::XMaskEvent(display, event_mask, event_return);
    }

    bool isEvent = false;
    for (int r=0; r<1000; r++) {
        isEvent = xlibEventQueue.pop(event_return, 0, event_mask);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents();
    }
    if (!isEvent) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    bool isEvent = xlibEventQueue.pop(event_return, 0, event_mask);
    return isEvent?True:False;
}

Bool XCheckTypedEvent(Display *display, int event_type, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedEvent);
        return orig::XCheckTypedEvent(display, event_type, event_return);
    }

    bool isEvent = xlibEventQueue.pop(event_return, 0, event_type);
    return isEvent?True:False;
}

Bool XCheckTypedWindowEvent(Display *display, Window w, int event_type, XEvent *event_return)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedWindowEvent);
        return orig::XCheckTypedWindowEvent(display, w, event_type, event_return);
    }

    bool isEvent = xlibEventQueue.pop(event_return, w, event_type);
    return isEvent?True:False;
}

int XEventsQueued(Display* display, int mode)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XEventsQueued);
        return orig::XEventsQueued(display, mode);
    }

    int ret = xlibEventQueue.size();
    debuglog(LCF_EVENTS, "    returns ", ret);
    return ret;
}

int XPending(Display *display)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XPending);
        return orig::XPending(display);
    }

    int ret = xlibEventQueue.size();
    debuglog(LCF_EVENTS, "    returns ", ret);
    return ret;
}

int XIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XIfEvent);
        return orig::XIfEvent(display, event_return, predicate, arg);
    }

    bool isEvent = false;
    for (int r=0; r<1000; r++) {
        isEvent = xlibEventQueue.pop(event_return, predicate, arg);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents();
    }
    if (!isEvent) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckIfEvent);
        return orig::XCheckIfEvent(display, event_return, predicate, arg);
    }

    bool isEvent = xlibEventQueue.pop(event_return, predicate, arg);
    return isEvent?True:False;
}

Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
    LINK_NAMESPACE_GLOBAL(XSendEvent);
    DEBUGLOGCALL(LCF_EVENTS);

    /* Detect and disable fullscreen switching */
    if (event_send->type == ClientMessage) {
        static Atom state = XInternAtom(display, "_NET_WM_STATE", True);
        static Atom state_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);

        if ((event_send->xclient.message_type == state) &&
            (event_send->xclient.data.l[0] == 1) &&
            (event_send->xclient.data.l[1] == state_fullscreen)) {
            debuglog(LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
            if (event_send->xclient.window != gameXWindow) {
                debuglog(LCF_EVENTS | LCF_WINDOW | LCF_WARNING, "   fullscreen window is not game window!");
            }
            XResizeWindow(display, event_send->xclient.window, XDisplayWidth(display, 0), XDisplayHeight(display, 0));
            return 0;
        }
    }

    return orig::XSendEvent(display, w, propagate, event_mask, event_send);
}

Bool XGetEventData(Display* dpy, XGenericEventCookie* cookie)
{
    DEBUGLOGCALL(LCF_EVENTS);
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XGetEventData);
        return orig::XGetEventData(dpy, cookie);
    }
    /* Data from our cookies are already present */
    if (cookie->type == GenericEvent)
        return True;
    return False;
}

void XFreeEventData(Display* dpy, XGenericEventCookie* cookie)
{
    DEBUGLOGCALL(LCF_EVENTS);
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XFreeEventData);
        orig::XFreeEventData(dpy, cookie);
    }
}


}
