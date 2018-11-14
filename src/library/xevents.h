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

#ifndef LIBTAS_XEVENTS_H_INCL
#define LIBTAS_XEVENTS_H_INCL

#include "global.h"
#include <X11/X.h>
#include <X11/Xlib.h>

namespace libtas {

OVERRIDE int XNextEvent(Display *display, XEvent *event_return);
OVERRIDE int XPeekEvent(Display *display, XEvent *event_return);
OVERRIDE int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
OVERRIDE Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return);
OVERRIDE int XMaskEvent(Display *display, long event_mask, XEvent *event_return);
OVERRIDE Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return);
OVERRIDE Bool XCheckTypedEvent(Display *display, int event_type, XEvent *event_return);
OVERRIDE Bool XCheckTypedWindowEvent(Display *display, Window w, int event_type, XEvent *event_return);
OVERRIDE int XEventsQueued(Display* display, int mode);
OVERRIDE int XPending(Display *display);
OVERRIDE Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send);

}

#endif
