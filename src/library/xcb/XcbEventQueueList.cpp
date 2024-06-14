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
#include "XcbEventQueueList.h"

#include "logging.h"

namespace libtas {

XcbEventQueueList xcbEventQueueList;

std::shared_ptr<XcbEventQueue> XcbEventQueueList::newQueue(xcb_connection_t *c)
{
    std::shared_ptr<XcbEventQueue> queue(new XcbEventQueue(c));
    eventQueueList.push_front(queue);
    return queue;
}

void XcbEventQueueList::deleteQueue(xcb_connection_t *c)
{
    for (auto it = eventQueueList.begin(); it != eventQueueList.end(); it++) {
        if ((*it)->c == c) {
            eventQueueList.erase(it);
            return;
        }
    }
}

std::shared_ptr<XcbEventQueue> XcbEventQueueList::getQueue(xcb_connection_t *c)
{
    for (auto queue: eventQueueList)
        if (queue->c == c)
            return queue;

    return nullptr;
}

int XcbEventQueueList::insert(xcb_connection_t *c, xcb_generic_event_t *event, bool full_event)
{
    for (auto queue: eventQueueList)
        if (queue->c == c)
            return queue->insert(event, full_event);

    return 0;
}

void XcbEventQueueList::insert(xcb_generic_event_t *event, bool full_event)
{
    for (auto queue: eventQueueList)
        queue->insert(event, full_event);
}

}
