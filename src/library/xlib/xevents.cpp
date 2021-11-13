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

#include "config.h"
#include "xevents.h"
#include "../logging.h"
#include "../hook.h"
#include "../GameHacks.h"
#include "../frame.h"
#include "XlibEventQueueList.h"
#include "xatom.h"
#include "xdisplay.h" // x11::gameDisplays
#include "xwindows.h" // x11::gameXWindows
#include "../../external/X11/XInput2.h"

#include "xrandr.h"

namespace libtas {

DEFINE_ORIG_POINTER(XCheckIfEvent)
DEFINE_ORIG_POINTER(XIfEvent)
DEFINE_ORIG_POINTER(XNextEvent)
DEFINE_ORIG_POINTER(XPeekEvent)
DEFINE_ORIG_POINTER(XWindowEvent)
DEFINE_ORIG_POINTER(XCheckWindowEvent)
DEFINE_ORIG_POINTER(XMaskEvent)
DEFINE_ORIG_POINTER(XCheckMaskEvent)
DEFINE_ORIG_POINTER(XCheckTypedEvent)
DEFINE_ORIG_POINTER(XCheckTypedWindowEvent)
DEFINE_ORIG_POINTER(XEventsQueued)
DEFINE_ORIG_POINTER(XPending)
DEFINE_ORIG_POINTER(XSendEvent)
DEFINE_ORIG_POINTER(XFlush)
DEFINE_ORIG_POINTER(XSync)
DEFINE_ORIG_POINTER(XGetEventData)
DEFINE_ORIG_POINTER(XFreeEventData)

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
        // case PropertyNotify:
        /* TODO: Re-enable this to filter unfocus events, but we must unfilter
         * this as Wine is looking for an event with WM_STATE atom */
        case ReparentNotify:
            return True;
        case ConfigureNotify:
            {
                XConfigureEvent* xce = reinterpret_cast<XConfigureEvent*>(event);
                xce->x = 0;
                xce->y = 0;
            }
            /* We need to filter this event on Unity. Without this, moving the
             * game window makes inputs stop registering. We still need this
             * on the first frame though.  */
            if (GameHacks::isUnity()) {
                if (framecount == 0)
                    return False;
                return True;                
            }
            return False;
        case ClientMessage:
            if (static_cast<Atom>(reinterpret_cast<XClientMessageEvent*>(event)->data.l[0]) == x11_atom(WM_TAKE_FOCUS))
                return True;
            return False;
        default:
            return False;
    }
}

void pushNativeXlibEvents(void)
{
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    for (int i=0; i<GAMEDISPLAYNUM; i++)
        if (x11::gameDisplays[i])
            pushNativeXlibEvents(x11::gameDisplays[i]);
}

void pushNativeXlibEvents(Display *display)
{
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    LINK_NAMESPACE_GLOBAL(XSync);
    LINK_NAMESPACE_GLOBAL(XPending);
    LINK_NAMESPACE_GLOBAL(XNextEvent);

    XLockDisplay(display);
    int n;
    NOLOGCALL(n = orig::XPending(display));
    while (n > 0) {
        XEvent event;
        NOLOGCALL(orig::XNextEvent(display, &event));

        if (event.type == ClientMessage) {
            /* Catch the close event */
            if (static_cast<Atom>(event.xclient.data.l[0]) == x11_atom(WM_DELETE_WINDOW)) {
                debuglogstdio(LCF_EVENTS | LCF_WINDOW, "    caught a window close event");
                is_exiting = true;
            }

            /* Catch a ping event */
            if ((event.xclient.message_type == x11_atom(WM_PROTOCOLS)) &&
                (static_cast<Atom>(event.xclient.data.l[0]) == x11_atom(_NET_WM_PING))) {

                debuglogstdio(LCF_EVENTS | LCF_WINDOW, "Answering a ping message");
                XEvent reply = event;
                reply.xclient.window = DefaultRootWindow(display);
                NATIVECALL(XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureNotifyMask | SubstructureRedirectMask, &reply));
            }
        }

        if (!isEventFiltered(&event)) {
            xlibEventQueueList.insert(display, &event);
        }

        NOLOGCALL(n = orig::XPending(display));
    }
    XUnlockDisplay(display);
}

int XNextEvent(Display *display, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XNextEvent);
        return orig::XNextEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XNextEvent);
        return orig::XNextEvent(display, event_return);
    }

    bool isEvent = false;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    for (int r=0; r<1000; r++) {
        isEvent = queue->pop(event_return, true);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents(display);
    }
    if (!isEvent) {
        debuglogstdio(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
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
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XPeekEvent);
        return orig::XPeekEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XPeekEvent);
        return orig::XPeekEvent(display, event_return);
    }

    bool isEvent = false;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    for (int r=0; r<1000; r++) {
        isEvent = queue->pop(event_return, false);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents(display);
    }
    if (!isEvent) {
        debuglogstdio(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XWindowEvent);
        return orig::XWindowEvent(display, w, event_mask, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XWindowEvent);
        return orig::XWindowEvent(display, w, event_mask, event_return);
    }

    bool isEvent = false;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    for (int r=0; r<1000; r++) {
        isEvent = queue->pop(event_return, w, event_mask);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents(display);
    }
    if (!isEvent) {
        debuglogstdio(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    bool isEvent = queue->pop(event_return, w, event_mask);
    return isEvent?True:False;
}

int XMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XMaskEvent);
        return orig::XMaskEvent(display, event_mask, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XMaskEvent);
        return orig::XMaskEvent(display, event_mask, event_return);
    }

    bool isEvent = false;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    for (int r=0; r<1000; r++) {
        isEvent = queue->pop(event_return, 0, event_mask);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents(display);
    }
    if (!isEvent) {
        debuglogstdio(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    bool isEvent = queue->pop(event_return, 0, event_mask);
    if (!isEvent) {
        pushNativeXlibEvents(display);
        isEvent = queue->pop(event_return, 0, event_mask);        
    }
    return isEvent?True:False;
}

Bool XCheckTypedEvent(Display *display, int event_type, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedEvent);
        return orig::XCheckTypedEvent(display, event_type, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedEvent);
        return orig::XCheckTypedEvent(display, event_type, event_return);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    bool isEvent = queue->pop(event_return, 0, event_type);
    if (!isEvent) {
        pushNativeXlibEvents(display);
        isEvent = queue->pop(event_return, 0, event_type);        
    }
    return isEvent?True:False;
}

Bool XCheckTypedWindowEvent(Display *display, Window w, int event_type, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedWindowEvent);
        return orig::XCheckTypedWindowEvent(display, w, event_type, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckTypedWindowEvent);
        return orig::XCheckTypedWindowEvent(display, w, event_type, event_return);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    bool isEvent = queue->pop(event_return, w, event_type);
    if (!isEvent) {
        pushNativeXlibEvents(display);
        isEvent = queue->pop(event_return, w, event_type);        
    }
    return isEvent?True:False;
}

int XEventsQueued(Display* display, int mode)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XEventsQueued);
        return orig::XEventsQueued(display, mode);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XEventsQueued);
        return orig::XEventsQueued(display, mode);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    int ret = queue->size();
    debuglogstdio(LCF_EVENTS, "    returns %d", ret);
    if ((ret == 0) && (mode != QueuedAlready))
        pushNativeXlibEvents(display);

    return ret;
}

int XPending(Display *display)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XPending);
        return orig::XPending(display);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XPending);
        return orig::XPending(display);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    int ret = queue->size();
    debuglogstdio(LCF_EVENTS, "    returns %d", ret);
    if (ret == 0)
        pushNativeXlibEvents(display);
    return ret;
}

int XIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XIfEvent);
        return orig::XIfEvent(display, event_return, predicate, arg);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XIfEvent);
        return orig::XIfEvent(display, event_return, predicate, arg);
    }

    bool isEvent = false;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    for (int r=0; r<1000; r++) {
        isEvent = queue->pop(event_return, predicate, arg);
        if (isEvent)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXlibEvents(display);
    }
    if (!isEvent) {
        debuglogstdio(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckIfEvent);
        return orig::XCheckIfEvent(display, event_return, predicate, arg);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XCheckIfEvent);
        return orig::XCheckIfEvent(display, event_return, predicate, arg);
    }

    pushNativeXlibEvents(display);

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    bool isEvent = queue->pop(event_return, predicate, arg);
    return isEvent?True:False;
}

Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
    LINK_NAMESPACE_GLOBAL(XSendEvent);

    if (GlobalState::isNative())
        return orig::XSendEvent(display, w, propagate, event_mask, event_send);

    DEBUGLOGCALL(LCF_EVENTS);

    /* Detect and disable several window state changes */
    if (event_send->type == ClientMessage) {
        if ((event_send->xclient.message_type == x11_atom(_NET_WM_STATE)) &&
            (event_send->xclient.data.l[0] == 1 /*_NET_WM_STATE_ADD*/ )) {

            /* Detect and disable fullscreen switching */
            if (static_cast<Atom>(event_send->xclient.data.l[1]) == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                debuglogstdio(LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                if (!x11::gameXWindows.empty() && (event_send->xclient.window != x11::gameXWindows.front())) {
                    debuglogstdio(LCF_EVENTS | LCF_WINDOW | LCF_WARNING, "   fullscreen window is not game window!");
                }

                /* Resize the window to the screen or fake resolution */
                if (shared_config.screen_width) {
                    XResizeWindow(display, event_send->xclient.window, shared_config.screen_width, shared_config.screen_height);
                }
                else {
                    /* Change the window size to monitor size */
                    int fs_width, fs_height;
                    get_monitor_resolution(fs_width, fs_height);
                    XResizeWindow(display, event_send->xclient.window, fs_width, fs_height);
                }
                return 0;
            }

            /* Detect and disable window always on top */
            if (static_cast<Atom>(event_send->xclient.data.l[1]) == x11_atom(_NET_WM_STATE_ABOVE)) {
                debuglogstdio(LCF_EVENTS | LCF_WINDOW, "   prevented window always on top");
                return 0;
            }
        }
    }

    return orig::XSendEvent(display, w, propagate, event_mask, event_send);
}

int XFlush(Display *display)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XFlush);
        return orig::XFlush(display);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XFlush);
        return orig::XFlush(display);
    }

    pushNativeXlibEvents(display);
    return 0;
}

int XSync(Display *display, Bool discard)
{
    LINK_NAMESPACE_GLOBAL(XSync);

    if (GlobalState::isNative()) {
        return orig::XSync(display, discard);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return orig::XSync(display, discard);
    }

    int ret = orig::XSync(display, discard);
    pushNativeXlibEvents(display);
    return ret;
}

Bool XGetEventData(Display* dpy, XGenericEventCookie* cookie)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XGetEventData);
        return orig::XGetEventData(dpy, cookie);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XGetEventData);
        return orig::XGetEventData(dpy, cookie);
    }

    /* Data from our cookies are already present */
    if (cookie->type == GenericEvent)
        return True;

    /* Make sure the data pointer is null, so that our `XFreeEventData()`
     * function can correctly detect non-null cookies. */
    cookie->data = nullptr;
    return False;
}

void XFreeEventData(Display* dpy, XGenericEventCookie* cookie)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XFreeEventData);
        return orig::XFreeEventData(dpy, cookie);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XFreeEventData);
        return orig::XFreeEventData(dpy, cookie);
    }

    if (cookie && cookie->data) {
        XIEvent* xiev = static_cast<XIEvent*>(cookie->data);
        XIRawEvent *rev;
        XIDeviceEvent* dev;
        switch(xiev->evtype) {
            case XI_RawMotion:
            case XI_RawKeyPress:
            case XI_RawKeyRelease:
            case XI_RawButtonPress:
            case XI_RawButtonRelease:
                rev = static_cast<XIRawEvent*>(cookie->data);
                /* Free allocated memory of XIRawEvent */
                if (rev->raw_values)
                    free(rev->raw_values);
                if (rev->valuators.values)
                    free(rev->valuators.values);
                if (rev->valuators.mask)
                    free(rev->valuators.mask);
                break;
            case XI_Motion:
            case XI_KeyPress:
            case XI_KeyRelease:
            case XI_ButtonPress:
            case XI_ButtonRelease:
                dev = static_cast<XIDeviceEvent*>(cookie->data);
                /* Free allocated memory of XIRawEvent */
                if (dev->buttons.mask)
                    free(dev->buttons.mask);
                break;
        }
        free(cookie->data);
    }
}


}
