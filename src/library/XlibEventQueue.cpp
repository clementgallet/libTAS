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

#include "XlibEventQueue.h"
#include "logging.h"

#ifdef LIBTAS_HAS_XINPUT
#include <X11/extensions/XInput2.h>
#endif

namespace libtas {

XlibEventQueue xlibEventQueue;

XlibEventQueue::~XlibEventQueue()
{
    for (auto ev: eventQueue) {
        delete ev;
    }
}

#define EVENTQUEUE_MAXLEN 1024

int XlibEventQueue::insert(XEvent* event)
{
    /* Check the size of the queue */
    if (eventQueue.size() > 1024) {
        debuglog(LCF_EVENTS, "We reached the limit of the event queue size!");
        return -1;
    }

    /* Building a dynamically allocated event */
    XEvent* ev = new XEvent;
    memcpy(ev, event, sizeof(XEvent));

    /* Push the event at the end of the queue */
    eventQueue.push_back(ev);

    return 1;
}

bool XlibEventQueue::pop(XEvent* event, bool update)
{
    if (eventQueue.size() == 0)
        return false;

    XEvent* ev = eventQueue.back();
    memcpy(event, ev, sizeof(XEvent));
    if (update) {
        delayedDeleteCookie(ev);
        delete ev;
        eventQueue.pop_back();
    }
    return true;
}

bool XlibEventQueue::pop(XEvent* event, Window w, long event_mask)
{
    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent* ev = *it;
        int type = ev->type;

        /* Check window match */
        if ((w != 0) && (w != ev->xany.window))
            continue;

        /* Check if event type match the mask */
        if ((type == MotionNotify) && !(event_mask & ButtonMotionMask)) continue;
        else if ((type == ButtonPress) && !(event_mask & ButtonPressMask)) continue;
        else if ((type == ButtonRelease) && !(event_mask & ButtonReleaseMask)) continue;
        else if ((type == ColormapNotify) && !(event_mask & ColormapChangeMask)) continue;
        else if ((type == EnterNotify) && !(event_mask & EnterWindowMask)) continue;
        else if ((type == LeaveNotify) && !(event_mask & LeaveWindowMask)) continue;
        else if ((type == Expose) && !(event_mask & ExposureMask)) continue;
        else if ((type == FocusIn) && !(event_mask & FocusChangeMask)) continue;
        else if ((type == FocusOut) && !(event_mask & FocusChangeMask)) continue;
        else if ((type == KeymapNotify) && !(event_mask & KeymapStateMask)) continue;
        else if ((type == KeyPress) && !(event_mask & KeyPressMask)) continue;
        else if ((type == KeyRelease) && !(event_mask & KeyReleaseMask)) continue;
        else if ((type == MotionNotify) && !(event_mask & PointerMotionMask)) continue;
        else if ((type == PropertyNotify) && !(event_mask & PropertyChangeMask)) continue;
        else if ((type == ResizeRequest) && !(event_mask & ResizeRedirectMask)) continue;
        else if ((type == CirculateNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == ConfigureNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == DestroyNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == GravityNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == MapNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == ReparentNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == UnmapNotify) && !(event_mask & (StructureNotifyMask|SubstructureNotifyMask))) continue;
        else if ((type == CirculateRequest) && !(event_mask & SubstructureRedirectMask)) continue;
        else if ((type == ConfigureRequest) && !(event_mask & SubstructureRedirectMask)) continue;
        else if ((type == MapRequest) && !(event_mask & SubstructureRedirectMask)) continue;
        else if ((type == VisibilityNotify) && !(event_mask & VisibilityChangeMask)) continue;

        /* We found a match */
        memcpy(event, ev, sizeof(XEvent));
        delayedDeleteCookie(ev);
        delete ev;
        eventQueue.erase(it);
        return true;
    }
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Window w, int event_type)
{
    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent* ev = *it;

        /* Check window match */
        if ((w != 0) && (w != ev->xany.window))
            continue;

        /* Check if event type match */
        if (ev->type != event_type)
            continue;

        /* We found a match */
        memcpy(event, ev, sizeof(XEvent));
        delayedDeleteCookie(ev);
        delete ev;
        eventQueue.erase(it);
        return true;
    }
    return false;
}

bool XlibEventQueue::pop(XEvent* event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg)
{
    for (auto it = eventQueue.begin(); it != eventQueue.end(); ++it) {
        XEvent* ev = *it;

        /* Check the predicate */
        if (predicate(ev->xany.display, ev, arg)) {
            /* We found a match */
            memcpy(event, ev, sizeof(XEvent));
            delayedDeleteCookie(ev);
            delete ev;
            eventQueue.erase(it);
            return true;
        }
    }
    return false;
}

int XlibEventQueue::size()
{
    return eventQueue.size();
}

void XlibEventQueue::delayedDeleteCookie(XEvent* event)
{
#ifdef LIBTAS_HAS_XINPUT
    if (cookieData) {
        XIEvent* xiev = static_cast<XIEvent*>(cookieData);
        XIRawEvent *rev;
        XIDeviceEvent* dev;
        switch(xiev->evtype) {
            case XI_RawMotion:
            case XI_RawKeyPress:
            case XI_RawKeyRelease:
            case XI_RawButtonPress:
            case XI_RawButtonRelease:
                rev = static_cast<XIRawEvent*>(cookieData);
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
                dev = static_cast<XIDeviceEvent*>(cookieData);
                /* Free allocated memory of XIRawEvent */
                if (dev->buttons.mask)
                    free(dev->buttons.mask);
                break;
        }
        free(cookieData);
    }
    if (event->type == GenericEvent) {
        cookieData = event->xcookie.data;
    }
}
#endif
}
