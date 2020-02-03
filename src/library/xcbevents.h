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

#ifndef LIBTAS_XCBEVENTS_H_INCL
#define LIBTAS_XCBEVENTS_H_INCL

#include "global.h"
#include <xcb/xcb.h>

namespace libtas {

/* Pull all events from the xcb event queue and push them into our
 * emulated event queue, filtering unwanted events (input events mainly).
 */
void pushNativeXcbEvents(void);
void pushNativeXcbEvents(xcb_connection_t *c);

// /* Wait for the xevent queue to become empty (returns true) or timeout (returns false) */
// bool syncXEvents();

OVERRIDE xcb_generic_event_t *xcb_wait_for_event(xcb_connection_t *c);
OVERRIDE xcb_generic_event_t *xcb_poll_for_event(xcb_connection_t *c);
// xcb_generic_event_t *xcb_poll_for_queued_event(xcb_connection_t *c);

OVERRIDE xcb_void_cookie_t
xcb_send_event_checked (xcb_connection_t *c,
                        uint8_t           propagate,
                        xcb_window_t      destination,
                        uint32_t          event_mask,
                        const char       *event);

OVERRIDE xcb_void_cookie_t
xcb_send_event (xcb_connection_t *c,
                uint8_t           propagate,
                xcb_window_t      destination,
                uint32_t          event_mask,
                const char       *event);

OVERRIDE int xcb_flush(xcb_connection_t *c);


}

#endif
