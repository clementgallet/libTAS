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
#include "XlibEventQueueList.h"
#include "../logging.h"
#include "../GlobalState.h"

namespace libtas {

XlibEventQueueList xlibEventQueueList;

std::shared_ptr<XlibEventQueue> XlibEventQueueList::newQueue(Display* display)
{
    std::shared_ptr<XlibEventQueue> queue(new XlibEventQueue(display));
    eventQueueList.push_front(queue);
    return queue;
}

void XlibEventQueueList::deleteQueue(Display* display)
{
    for (auto it = eventQueueList.begin(); it != eventQueueList.end(); it++) {
        if ((*it)->display == display) {
            eventQueueList.erase(it);
            return;
        }
    }
}

std::shared_ptr<XlibEventQueue> XlibEventQueueList::getQueue(Display* display)
{
    for (auto queue: eventQueueList)
        if (queue->display == display)
            return queue;

    return nullptr;
}

int XlibEventQueueList::insert(Display* display, XEvent* event)
{
    for (auto queue: eventQueueList)
        if (queue->display == display)
            return queue->insert(event);

    return 0;
}

void XlibEventQueueList::insert(XEvent* event)
{
    for (auto queue: eventQueueList)
        queue->insert(event);
}

bool XlibEventQueueList::waitForEmpty()
{
    int attempts = 0;
    bool allEmpty = false;

    while (!allEmpty) {
        allEmpty = true;
        for (auto queue: eventQueueList) {
            allEmpty &= queue->emptied;
            if (!queue->emptied) {
                if (++attempts > 10 * 100) {
                    debuglogstdio(LCF_EVENTS | LCF_ERROR | LCF_ALERT, "xevents sync took too long, were asynchronous events incorrectly enabled?");
                    return false;
                }
            }
        }
        struct timespec sleepTime = { 0, 10 * 1000 };
        NATIVECALL(nanosleep(&sleepTime, NULL));
    }
    return true;
}

/* Reset the empty state of each queue */
void XlibEventQueueList::resetEmpty()
{
    for (auto queue: eventQueueList)
        queue->emptied = false;
}


void XlibEventQueueList::lock()
{
    for (auto queue: eventQueueList)
        queue->mutex.lock();
}

void XlibEventQueueList::unlock()
{
    for (auto queue: eventQueueList)
        queue->mutex.unlock();
}

}
