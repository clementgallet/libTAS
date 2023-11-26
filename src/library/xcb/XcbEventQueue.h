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

#ifndef LIBTAS_XCBEVENTQUEUE_H_INCLUDED
#define LIBTAS_XCBEVENTQUEUE_H_INCLUDED

#include <list>
#include <map>
#include <xcb/xcb.h>

namespace libtas {
/* This is a replacement of the xcb event queue. */
class XcbEventQueue
{
    public:
        XcbEventQueue(xcb_connection_t *c);

        /* Set an event mask for a window */
        void setMask(xcb_window_t wid, uint32_t event_mask);

        /* Insert an event in the queue */
        int insert(xcb_generic_event_t *event);

        /* Return a copy of the first event of the queue, and remove it from
         * the queue. */
        xcb_generic_event_t* pop();

        /* Return the size of the queue */
        int size();

        xcb_connection_t *c;

    private:
        /* Event queue */
        std::list<xcb_generic_event_t> eventQueue;

        /* Event mask for each Window */
        std::map<xcb_window_t, uint32_t> eventMasks;
};

}

#endif
