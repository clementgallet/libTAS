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

#ifndef LIBTAS_XCBEVENTQUEUELIST_H_INCLUDED
#define LIBTAS_XCBEVENTQUEUELIST_H_INCLUDED

#include "XcbEventQueue.h"

#include <list>
#include <memory>
#include <xcb/xcb.h>

namespace libtas {
/* This class stores all xcb event queues. */
class XcbEventQueueList
{
    public:
        /* Create a new queue associated with a connection */
        std::shared_ptr<XcbEventQueue> newQueue(xcb_connection_t *c);

        /* Delete a new queue associated with a connection */
        void deleteQueue(xcb_connection_t *c);

        /* Return the queue associated with a connection */
        std::shared_ptr<XcbEventQueue> getQueue(xcb_connection_t *c);

        /* Insert an event into a specific queue */
        int insert(xcb_connection_t *c, xcb_generic_event_t *event);

        /* Insert an event into all queues */
        void insert(xcb_generic_event_t *event);

    private:
        std::list<std::shared_ptr<XcbEventQueue>> eventQueueList;

};

extern XcbEventQueueList xcbEventQueueList;

}

#endif
