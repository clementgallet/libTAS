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

#include "config.h"
#include "XlibEventQueue.h"
#include "xwindows.h"

#include "logging.h"
#include "inputs/inputs.h"

namespace libtas {

XlibEventQueue::XlibEventQueue(Display* d) : display(d), sync_count(0), grab_window(0), grab_event_mask(0), grab_owner_events(false), focus_window(0) {}

void XlibEventQueue::setMask(Window w, long event_mask)
{
    long old_event_mask = eventMasks[w];
    eventMasks[w] = event_mask;

    /* If the game is interested in the EnterNotify event for the first time,
     * send one immediately */
    if (event_mask & EnterWindowMask && (!(old_event_mask & EnterWindowMask))) {
        XEvent ev;
        ev.type = EnterNotify;
        ev.xcrossing.window = w;
        ev.xcrossing.x = Inputs::game_ai.pointer.x;
        ev.xcrossing.y = Inputs::game_ai.pointer.y;
        ev.xcrossing.x_root = Inputs::game_ai.pointer.x;
        ev.xcrossing.y_root = Inputs::game_ai.pointer.y;
        ev.xcrossing.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        ev.xcrossing.send_event = 0;
        ev.xcrossing.same_screen = 1;
        ev.xcrossing.root = DefaultRootWindow(display);
        ev.xcrossing.mode = NotifyNormal;
        ev.xcrossing.detail = NotifyNonlinear;

        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "   Inserting a EnterNotify event for window %d", w);
        insert(&ev);
    }

    /* If the game is interested in the FocusIn event for the first time,
     * send one immediately */
    if (event_mask & FocusChangeMask && (!(old_event_mask & FocusChangeMask))) {
        XEvent ev;
        ev.type = FocusIn;
        ev.xfocus.window = w;
        ev.xfocus.mode = grab_window ? NotifyWhileGrabbed : NotifyNormal;

        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE | LCF_KEYBOARD, "   Inserting a FocusIn event");
        insert(&ev);
    }
}

#define EVENTQUEUE_MAXLEN 1024

int XlibEventQueue::insert(XEvent* event)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    /* Redirect events to the currently focused keyboard window, mirroring
     * XSetInputFocus semantics for the emulated event queue. */
    if ((focus_window != 0) &&
        ((event->type == KeyPress) || (event->type == KeyRelease) ||
         (event->type == FocusIn) || (event->type == FocusOut) ||
         (event->type == KeymapNotify))) {
        if (event->type == FocusIn || event->type == FocusOut) {
            event->xfocus.window = focus_window;
        }
        else if (event->type == KeyPress || event->type == KeyRelease) {
            event->xkey.window = focus_window;
        }
        else {
            event->xkey.window = focus_window;
        }
        event->xany.window = focus_window;
    }

    /* Check if pointer event and pointer is grabbed */
    if ((grab_window != 0) &&
        ((event->type == MotionNotify) || (event->type == ButtonPress) ||
         (event->type == ButtonRelease) || (event->type == EnterNotify) ||
         (event->type == LeaveNotify) || (event->type == KeymapNotify))) {
             
        /* Check grab mask */
        if (isTypeOfMask(event->type, grab_event_mask)) {
            /* Register event to the grab window */
            event->xany.window = grab_window;
            
            /* Check the size of the queue */
            if (eventQueue.size() > 1024) {
                LOG(LL_DEBUG, LCF_EVENTS, "We reached the limit of the event queue size!");
                return -1;
            }

            /* Specify the display */
            event->xany.display = display;

            /* Push the event at the beginning of the queue */
            eventQueue.push_front(*event);

            /* If grab was set with owner_events being False, only report to 
             * the grab window, or discard */
            if (!grab_owner_events)
                return 1;
        }
        else {
            if (!grab_owner_events)
                return 0;
        }
    }
    
    /* Check if the window can produce such event */
    
    /* We are supposed to check if the parent window has SubstructureNotifyMask
     * so that current window can receive ConfigureNotify events, but I'm lazy,
     * so we are always returning those events */

    if (event->type != ConfigureNotify) {

        LOG(LL_DEBUG, LCF_EVENTS, "Check mask of window %d", event->xany.window);
        /* Find the window in the hierarchy that is interested in this event */
        while (event->xany.window != 0) {
            auto mask = eventMasks.find(event->xany.window);
            if (mask != eventMasks.end() && isTypeOfMask(event->type, mask->second)) {
                break;
            }
            /* Check unmaskable events */
            else if (isTypeOfMask(event->type, 0)) {
                break;
            }
            event->xany.window = getParentWindow(event->xany.window);
            LOG(LL_DEBUG, LCF_EVENTS, "Check parent window %d", event->xany.window);
        }
        if (!event->xany.window)
            return 0;
        LOG(LL_DEBUG, LCF_EVENTS, "Found interested window %d", event->xany.window);
    }

    /* Check the size of the queue */
    if (eventQueue.size() > 1024) {
        LOG(LL_DEBUG, LCF_EVENTS, "We reached the limit of the event queue size!");
        return -1;
    }

    /* Specify the display */
    event->xany.display = display;

    /* Push the event at the beginning of the queue */
    eventQueue.push_front(*event);

    return 1;
}

bool XlibEventQueue::pop(XEvent* event, bool update)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    if (eventQueue.size() == 0) {
        return false;
    }

    XEvent ev = eventQueue.back();
    memcpy(event, &ev, sizeof(XEvent));
    if (update) {
        eventQueue.pop_back();
    }
    return true;
}

bool XlibEventQueue::pop(XEvent* event, Window w, long event_mask)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent ev = *it;
        int type = ev.type;

        /* Check window match */
        if ((w != 0) && (w != ev.xany.window))
            continue;

        /* Check if event type match the mask */
        if (!isTypeOfMask(type, event_mask))
            continue;

        /* We found a match */
        memcpy(event, &ev, sizeof(XEvent));
        eventQueue.erase(it);
        return true;
    }
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Window w, int event_type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent ev = *it;

        /* Check window match */
        if ((w != 0) && (w != ev.xany.window))
            continue;

        /* Check if event type match */
        if (ev.type != event_type)
            continue;

        /* We found a match */
        memcpy(event, &ev, sizeof(XEvent));
        eventQueue.erase(it);
        return true;
    }
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent ev = *it;

        /* Check the predicate */
        if (predicate(ev.xany.display, &ev, arg)) {
            /* We found a match */
            memcpy(event, &ev, sizeof(XEvent));
            eventQueue.erase(it);
            return true;
        }
    }
    return false;
}

int XlibEventQueue::size()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    size_t s = eventQueue.size();
    return s;
}

void XlibEventQueue::grabPointer(Window window, unsigned int event_mask, bool owner_events)
{
    grab_window = window;
    grab_event_mask = event_mask;
    grab_owner_events = owner_events;
}

void XlibEventQueue::ungrabPointer()
{
    grab_window = 0;
}

void XlibEventQueue::setInputFocus(Window window)
{
    focus_window = window;

    long event_mask = eventMasks[window];
    if (event_mask & FocusChangeMask) {
        XEvent ev;
        ev.type = FocusIn;
        ev.xfocus.window = window;
        ev.xfocus.mode = grab_window ? NotifyWhileGrabbed : NotifyNormal;

        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE | LCF_KEYBOARD, "   Inserting a FocusIn event");
        insert(&ev);
    }
}

void XlibEventQueue::clearInputFocus()
{
    focus_window = 0;
}

Window XlibEventQueue::getInputFocus() const
{
    return focus_window;
}

bool XlibEventQueue::isTypeOfMask(int type, long event_mask)
{
    switch (type) {
        case MotionNotify:
            return event_mask & (ButtonMotionMask|PointerMotionMask);
        case ButtonPress:
            return event_mask & ButtonPressMask;
        case ButtonRelease:
            return event_mask & ButtonReleaseMask;
        case ColormapNotify:
            return event_mask & ColormapChangeMask;
        case EnterNotify:
            return event_mask & EnterWindowMask;
        case LeaveNotify:
            return event_mask & LeaveWindowMask;
        case Expose:
            return event_mask & ExposureMask;
        case FocusIn:
            return event_mask & FocusChangeMask;
        case FocusOut:
            return event_mask & FocusChangeMask;
        case KeymapNotify:
            return event_mask & KeymapStateMask;
        case KeyPress:
            return event_mask & KeyPressMask;
        case KeyRelease:
            return event_mask & KeyReleaseMask;
        case PropertyNotify:
            return event_mask & PropertyChangeMask;
        case ResizeRequest:
            return event_mask & ResizeRedirectMask;
        case CirculateNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case ConfigureNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case DestroyNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case GravityNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case MapNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case ReparentNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case UnmapNotify:
            return event_mask & (StructureNotifyMask|SubstructureNotifyMask);
        case CirculateRequest:
            return event_mask & SubstructureRedirectMask;
        case ConfigureRequest:
            return event_mask & SubstructureRedirectMask;
        case MapRequest:
            return event_mask & SubstructureRedirectMask;
        case VisibilityNotify:
            return event_mask & VisibilityChangeMask;
        default:
            /* If no mask, it's an unmaskable event */
            return true;
    }
    return true;
}

Window XlibEventQueue::getParentWindow(Window w)
{
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    Window root_window;
    if (!XQueryTree(display, w, &root_window, &parent_return, &children_return, &nchildren_return)) {
        return 0;
    }
    if (children_return) XFree(children_return);

    return (parent_return != DefaultRootWindow(display)) ? parent_return : 0;
}

} // namespace libtas