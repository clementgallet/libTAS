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
#include "XlibEventQueue.h"
#include "../logging.h"
#include "../inputs/inputs.h"
#include "xwindows.h"

namespace libtas {

XlibEventQueue::XlibEventQueue(Display* d) : display(d), emptied(false) {}

void XlibEventQueue::setMask(Window w, long event_mask)
{
    /* If the game is interested in the EnterNotify event for the first time,
     * send one immediately */
    if (event_mask & EnterWindowMask && (!(eventMasks[w] & EnterWindowMask))) {
        XEvent ev;
        ev.type = EnterNotify;
        ev.xcrossing.window = w;
        ev.xcrossing.x = game_ai.pointer_x;
        ev.xcrossing.y = game_ai.pointer_y;
        ev.xcrossing.x_root = game_ai.pointer_x;
        ev.xcrossing.y_root = game_ai.pointer_y;
        ev.xcrossing.state = SingleInput::toXlibPointerMask(ai.pointer_mask);
        ev.xcrossing.send_event = 0;
        ev.xcrossing.same_screen = 1;
        ev.xcrossing.root = x11::rootWindow;
        ev.xcrossing.mode = NotifyNormal;
        ev.xcrossing.detail = NotifyNonlinear;

        debuglogstdio(LCF_EVENTS | LCF_MOUSE, "   Inserting a EnterNotify event for window %d", w);
        insert(&ev);
    }

    /* If the game is interested in the FocusIn event for the first time,
     * send one immediately */
    if (event_mask & FocusChangeMask && (!(eventMasks[w] & FocusChangeMask))) {
        XEvent ev;
        ev.type = FocusIn;
        ev.xfocus.window = w;

        debuglogstdio(LCF_EVENTS | LCF_MOUSE | LCF_KEYBOARD, "   Inserting a FocusIn event");
        insert(&ev);
    }

    eventMasks[w] = event_mask;
}

#define EVENTQUEUE_MAXLEN 1024

int XlibEventQueue::insert(XEvent* event)
{
    /* Check if the window can produce such event */
    auto mask = eventMasks.find(event->xany.window);
    if (mask != eventMasks.end()) {
        if (!isTypeOfMask(event->xany.type, mask->second))
            return 0;
    }
    else {
        /* Check unmaskable events */
        if (!isTypeOfMask(event->xany.type, 0))
            return 0;
    }

    /* Check the size of the queue */
    if (eventQueue.size() > 1024) {
        debuglogstdio(LCF_EVENTS, "We reached the limit of the event queue size!");
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
    std::lock_guard<std::mutex> lock(mutex);

    if (eventQueue.size() == 0) {
        emptied = true;
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
    std::lock_guard<std::mutex> lock(mutex);

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
    emptied = true;
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Window w, int event_type)
{
    std::lock_guard<std::mutex> lock(mutex);

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
    emptied = true;
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    std::lock_guard<std::mutex> lock(mutex);

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
    emptied = true;
    return false;
}

int XlibEventQueue::size()
{
    std::lock_guard<std::mutex> lock(mutex);

    size_t s = eventQueue.size();
    if (s == 0)
        emptied = true;
    return s;
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


}
