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

namespace orig {
    static int (*XNextEvent)(Display *display, XEvent *event_return);
    static int (*XPending)(Display *display);
}

/* Return we must prevent an event from being read by the game */
static bool filterXEvent(XEvent *event)
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

int XNextEvent(Display *display, XEvent *event_return)
{
    LINK_NAMESPACE(XNextEvent, nullptr);
    if (GlobalState::isOwnCode()) {
        return orig::XNextEvent(display, event_return);
    }

    DEBUGLOGCALL(LCF_EVENTS);
    int ret = orig::XNextEvent(display, event_return);
    LINK_NAMESPACE(XPending, nullptr);

    /* Pull events from the queue until the first non-filtered event or
     * until the queue is empty.
     */
    while (filterXEvent(event_return) && !event_return->xkey.send_event &&
        (orig::XPending(display) > 0)) {
        /* This event is filtered, we pull the next one */
        ret = orig::XNextEvent(display, event_return);
    }

    if (filterXEvent(event_return) && !event_return->xkey.send_event) {
        /* We only pulled filtered events and the queue is empty. We must
         * return some event, so returning a dummy event.
         */
        event_return->type = GenericEvent;
    }

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

}
