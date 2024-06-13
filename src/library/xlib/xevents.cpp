/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "XlibEventQueueList.h"
#include "xatom.h"
#include "xdisplay.h" // x11::gameDisplays
#include "xwindows.h" // x11::gameXWindows
#include "xrandr.h"

#include "logging.h"
#include "hook.h"
#include "UnityHacks.h"
#include "frame.h"
#include "global.h"
#include "GlobalState.h"
#include "renderhud/RenderHUD.h"
#include "screencapture/ScreenCapture.h"
#include "../external/X11/XInput2.h"
#include "../external/imgui/imgui_impl_xlib.h"

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
DEFINE_ORIG_POINTER(XFilterEvent)
DEFINE_ORIG_POINTER(XFlush)
DEFINE_ORIG_POINTER(XSync)
DEFINE_ORIG_POINTER(XGetEventData)
DEFINE_ORIG_POINTER(XFreeEventData)


static void debugEvent(XEvent *event);

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
            if (UnityHacks::isUnity()) {
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
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    for (int i=0; i<GAMEDISPLAYNUM; i++)
        if (x11::gameDisplays[i])
            pushNativeXlibEvents(x11::gameDisplays[i]);
}

void pushNativeXlibEvents(Display *display)
{
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    LINK_NAMESPACE_GLOBAL(XSync);
    LINK_NAMESPACE_GLOBAL(XPending);
    LINK_NAMESPACE_GLOBAL(XNextEvent);

    NOLOGCALL(XLockDisplay(display));
    int n;
    NOLOGCALL(n = orig::XPending(display));
    while (n > 0) {
        XEvent event;
        NOLOGCALL(orig::XNextEvent(display, &event));

        if (event.type == ClientMessage) {
            /* Catch the close event */
            if (static_cast<Atom>(event.xclient.data.l[0]) == x11_atom(WM_DELETE_WINDOW)) {
                LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "    caught a window close event");
                Global::is_exiting = true;
            }

            /* Catch a ping event */
            if ((event.xclient.message_type == x11_atom(WM_PROTOCOLS)) &&
                (static_cast<Atom>(event.xclient.data.l[0]) == x11_atom(_NET_WM_PING))) {

                LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "Answering a ping message");
                XEvent reply = event;
                reply.xclient.window = DefaultRootWindow(display);
                NATIVECALL(XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureNotifyMask | SubstructureRedirectMask, &reply));
            }
        }
        
        if (ImGui::GetCurrentContext()) {
            GlobalNative gn;
            /* Redirect events to ImGui, and notify if it was interested in the event */
            if (ImGui_ImplXlib_ProcessEvent(&event))
                RenderHUD::userInputs();
                
            /* Detect window resizing by the user */
            if (event.type == ConfigureNotify) {
                XConfigureEvent xce = event.xconfigure;

                int w = 0, h = 0;
                ScreenCapture::getDimensions(w, h);

                if (w != xce.width || h != xce.height) {
                    /* Check the saved dimension to see if the event was generated
                     * by window resizing or API call */
                     
                    /* Detach the game window */
                    RenderHUD::detachGameWindow();

                    /* Skip the event */
                    NOLOGCALL(n = orig::XPending(display));
                    continue;
                }
                
                /* TODO: We need to do something to prevent games to know that
                 * we resized the game window */
                // xce.width = w;
                // xce.height = h;
            }
        }

        if (!isEventFiltered(&event)) {
            xlibEventQueueList.insert(display, &event);
        }

        NOLOGCALL(n = orig::XPending(display));
    }
    NOLOGCALL(XUnlockDisplay(display));
}

int XNextEvent(Display *display, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XNextEvent);
        return orig::XNextEvent(display, event_return);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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
        LOG(LL_WARN, LCF_EVENTS, "    waited too long for an event");
    }
    return 0;
}

int XPeekEvent(Display *display, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XPeekEvent);
        return orig::XPeekEvent(display, event_return);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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
        LOG(LL_WARN, LCF_EVENTS, "    waited too long for an event");
    }
    return 0;
}

int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XWindowEvent);
        return orig::XWindowEvent(display, w, event_mask, event_return);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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
        LOG(LL_WARN, LCF_EVENTS, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckWindowEvent);
        return orig::XCheckWindowEvent(display, w, event_mask, event_return);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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
        LOG(LL_WARN, LCF_EVENTS, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckMaskEvent);
        return orig::XCheckMaskEvent(display, event_mask, event_return);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XEventsQueued);
        return orig::XEventsQueued(display, mode);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    int ret = queue->size();
    LOG(LL_DEBUG, LCF_EVENTS, "    returns %d", ret);
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(XPending);
        return orig::XPending(display);
    }

    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    int ret = queue->size();
    LOG(LL_DEBUG, LCF_EVENTS, "    returns %d", ret);
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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
        
        if (Global::is_exiting)
            return 0;
    }
    if (!isEvent) {
        LOG(LL_WARN, LCF_EVENTS, "    waited too long for an event");
    }
    return 0;
}

Bool XCheckIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XCheckIfEvent);
        return orig::XCheckIfEvent(display, event_return, predicate, arg);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    /* Detect and disable several window state changes */
    if (event_send->type == ClientMessage) {
        if ((event_send->xclient.message_type == x11_atom(_NET_WM_STATE)) &&
            (event_send->xclient.data.l[0] == 1 /*_NET_WM_STATE_ADD*/ )) {

            /* Detect and disable fullscreen switching */
            if (static_cast<Atom>(event_send->xclient.data.l[1]) == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                if (!x11::gameXWindows.empty() && (event_send->xclient.window != x11::gameXWindows.front())) {
                    LOG(LL_WARN, LCF_EVENTS | LCF_WINDOW, "   fullscreen window is not game window!");
                }

                /* Resize the window to the screen or fake resolution */
                if (Global::shared_config.screen_width) {
                    XResizeWindow(display, event_send->xclient.window, Global::shared_config.screen_width, Global::shared_config.screen_height);
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
                LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "   prevented window always on top");
                return 0;
            }
        }
    }

    return orig::XSendEvent(display, w, propagate, event_mask, event_send);
}

Bool XFilterEvent(XEvent *event, Window w)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XFilterEvent);
        return orig::XFilterEvent(event, w);
    }
    
    LOGTRACE(LCF_EVENTS);
    /* This is used when using composition, but we don't support it.
     * In the meanwhile, we disable it completely. Users that want to input
     * special characters can map in libTAS a key to that character.
     */
    return False;
}

int XFlush(Display *display)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(XFlush);
        return orig::XFlush(display);
    }

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

    LOGTRACE(LCF_EVENTS);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
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

/* Show informations about an xevent */
static void debugEvent(XEvent *event)
{
    LOG(LL_DEBUG, LCF_EVENTS, "Debug event %p:", event);
    if (!event) return;

    LOG(LL_DEBUG, LCF_EVENTS, "| Generic information");
    
    LOG(LL_DEBUG, LCF_EVENTS, "| | type: %d", event->type);
    LOG(LL_DEBUG, LCF_EVENTS, "| | serial: %d", event->xany.serial);
    LOG(LL_DEBUG, LCF_EVENTS, "| | send_event: %d", event->xany.send_event);
    LOG(LL_DEBUG, LCF_EVENTS, "| | display: %p", event->xany.display);
    LOG(LL_DEBUG, LCF_EVENTS, "| |_window: %d", event->xany.window);

    LOG(LL_DEBUG, LCF_EVENTS, "| Specific information");
    switch(event->type) {
        case KeyPress:
        case KeyRelease:
            if (event->type == KeyPress)
                LOG(LL_DEBUG, LCF_EVENTS, "| | type: KeyPress");
            else
                LOG(LL_DEBUG, LCF_EVENTS, "| | type: KeyRelease");
            LOG(LL_DEBUG, LCF_EVENTS, "| | root: %d", event->xkey.root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | subwindow: %d", event->xkey.subwindow);
            LOG(LL_DEBUG, LCF_EVENTS, "| | time: %d", event->xkey.time);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x: %d", event->xkey.x);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y: %d", event->xkey.y);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x_root: %d", event->xkey.x_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y_root: %d", event->xkey.y_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | state: %u", event->xkey.state);
            LOG(LL_DEBUG, LCF_EVENTS, "| | keycode: %u", event->xkey.keycode);
            LOG(LL_DEBUG, LCF_EVENTS, "| |_same_screen: %d", event->xkey.same_screen);
            break;
        case ButtonPress:
        case ButtonRelease:
            if (event->type == ButtonPress)
                LOG(LL_DEBUG, LCF_EVENTS, "| | type: ButtonPress");
            else
                LOG(LL_DEBUG, LCF_EVENTS, "| | type: ButtonRelease");
            LOG(LL_DEBUG, LCF_EVENTS, "| | root: %d", event->xbutton.root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | subwindow: %d", event->xbutton.subwindow);
            LOG(LL_DEBUG, LCF_EVENTS, "| | time: %d", event->xbutton.time);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x: %d", event->xbutton.x);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y: %d", event->xbutton.y);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x_root: %d", event->xbutton.x_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y_root: %d", event->xbutton.y_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | state: %u", event->xbutton.state);
            LOG(LL_DEBUG, LCF_EVENTS, "| | button: %u", event->xbutton.button);
            LOG(LL_DEBUG, LCF_EVENTS, "| |_same_screen: %d", event->xbutton.same_screen);
            break;
        case MotionNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: MotionNotify");
            LOG(LL_DEBUG, LCF_EVENTS, "| | root: %d", event->xmotion.root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | subwindow: %d", event->xmotion.subwindow);
            LOG(LL_DEBUG, LCF_EVENTS, "| | time: %d", event->xmotion.time);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x: %d", event->xmotion.x);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y: %d", event->xmotion.y);
            LOG(LL_DEBUG, LCF_EVENTS, "| | x_root: %d", event->xmotion.x_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | y_root: %d", event->xmotion.y_root);
            LOG(LL_DEBUG, LCF_EVENTS, "| | state: %u", event->xmotion.state);
            LOG(LL_DEBUG, LCF_EVENTS, "| | is_hint: %d", event->xmotion.is_hint);
            LOG(LL_DEBUG, LCF_EVENTS, "| |_same_screen: %d", event->xmotion.same_screen);
            break;
        case GenericEvent:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: GenericEvent");
            LOG(LL_DEBUG, LCF_EVENTS, "| | extension: %d", event->xgeneric.extension);
            LOG(LL_DEBUG, LCF_EVENTS, "| |_evtype: %d", event->xgeneric.evtype);
            break;
        case FocusIn:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: FocusIn");
            break;
        case FocusOut:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: FocusOut");
            break;
        case Expose:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: Expose");
            break;
        case EnterNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: EnterNotify");
            break;
        case LeaveNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: LeaveNotify");
            break;
        case PropertyNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: PropertyNotify");
            break;
        case ReparentNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: ReparentNotify");
            break;
        case ConfigureNotify:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: ConfigureNotify");
            break;
        case ClientMessage:
            LOG(LL_DEBUG, LCF_EVENTS, "| | type: ClientMessage");
            break;
    }    
    LOG(LL_DEBUG, LCF_EVENTS, "|___");
}

}
