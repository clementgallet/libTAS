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

#include "SDLEventQueue.h"
#include "../logging.h"
// #include <string.h>
#include "../global.h" // game_info

namespace libtas {

SDLEventQueue sdlEventQueue;

SDLEventQueue::~SDLEventQueue()
{
    for (auto ev: eventQueue) {
        if (game_info.video & GameInfo::SDL1)
            delete static_cast<SDL1::SDL_Event*>(ev);
        if (game_info.video & GameInfo::SDL2)
            delete static_cast<SDL_Event*>(ev);
    }
}

void SDLEventQueue::init(void)
{
    emptied = false;
    if (game_info.video & GameInfo::SDL2) {
        /* Insert default filters */
        droppedEvents.insert(SDL_TEXTINPUT);
        droppedEvents.insert(SDL_TEXTEDITING);
        droppedEvents.insert(SDL_SYSWMEVENT);
    }
}

void SDLEventQueue::disable(int type)
{
    droppedEvents.insert(type);
}

void SDLEventQueue::enable(int type)
{
    std::set<int>::iterator it = droppedEvents.find(type);
    if (it != droppedEvents.end())
        droppedEvents.erase(it);
}

bool SDLEventQueue::isEnabled(int type)
{
    return droppedEvents.find(type) == droppedEvents.end();
}

#define EVENTQUEUE_MAXLEN 1024

int SDLEventQueue::insert(SDL_Event* event)
{
    /* Before inserting the event, we have some checks in a specific order */

    /* 1. Check that the event type is enabled */
    if (!isEnabled(event->type))
        return 0;

    /* 2. Run the event filter if set, and check the returned value */
    if (filterFunc != nullptr) {
        int isInserted = filterFunc(filterData, event);
        if (!isInserted)
            return 0;
    }

    /* 3. Call all watchers on the event */
    for (auto watch: watches) {
        watch.first(watch.second, event);
    }

    /* 4. Check the size of the queue */
    if (eventQueue.size() > 1024) {
        debuglogstdio(LCF_SDL | LCF_EVENTS, "We reached the limit of the event queue size!");
        return -1;
    }

    /* Building a dynamically allocated event */
    SDL_Event* ev = new SDL_Event;
    memcpy(ev, event, sizeof(SDL_Event));

    /* Push the event at the end of the queue */
    eventQueue.push_back(ev);

    return 1;
}

int SDLEventQueue::insert(SDL1::SDL_Event* event)
{
    /* Before inserting the event, we have some checks in a specific order */

    /* 1. Check that the event type is enabled */
    if (!isEnabled(event->type))
        return -1;

    /* 2. Run the event filter if set, and check the returned value */
    if (filterFunc1 != nullptr) {
        int isInserted = filterFunc1(event);
        if (!isInserted)
            return -1;
    }

    /* 3. Check the size of the queue */
    if (eventQueue.size() > 1024) {
        debuglogstdio(LCF_SDL | LCF_EVENTS, "We reached the limit of the event queue size!");
        return -1;
    }

    /* Building a dynamically allocated event */
    SDL1::SDL_Event* ev = new SDL1::SDL_Event;
    memcpy(ev, event, sizeof(SDL1::SDL_Event));

    /* Push the event at the end of the queue */
    eventQueue.push_back(ev);

    return 0;
}

int SDLEventQueue::pop(SDL_Event* events, int num, Uint32 minType, Uint32 maxType, bool update)
{
    std::lock_guard<std::mutex> lock(mutex);
    int evi = 0;

    if (num <= 0)
        return 0;

    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL_Event* ev = static_cast<SDL_Event*>(*it);

        /* Check if event match the filter */
        if ((ev->type >= minType) && (ev->type <= maxType)) {

            /* Copy the event in the array */
            memcpy(&events[evi], ev, sizeof(SDL_Event));
            evi++;

            if (update) {
                /* Deleting the object and removing it from the list */
                delete ev;
                it = eventQueue.erase(it);
            }
            else
                ++it;

            /* Check if we reached the limit on the number of events */
            if (evi >= num)
                return num;
        }
        else {
            ++it;
        }

    }

    emptied = true;
    return evi;

}

int SDLEventQueue::pop(SDL1::SDL_Event* events, int num, Uint32 mask, bool update)
{
    std::lock_guard<std::mutex> lock(mutex);
    int evi = 0;

    if (num <= 0)
        return 0;

    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL1::SDL_Event* ev = static_cast<SDL1::SDL_Event*>(*it);

        /* Check if event match the filter */
        if (mask & SDL1_EVENTMASK(ev->type)) {

            /* Copy the event in the array */
            memcpy(&events[evi], ev, sizeof(SDL1::SDL_Event));
            evi++;

            if (update) {
                /* Deleting the object and removing it from the list */
                delete ev;
                it = eventQueue.erase(it);
            }
            else
                ++it;

            /* Check if we reached the limit on the number of events */
            if (evi >= num)
                return num;

        }
        else {
            ++it;
        }
    }

    emptied = true;
    return evi;

}

void SDLEventQueue::flush(Uint32 minType, Uint32 maxType)
{
    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL_Event* ev = static_cast<SDL_Event*>(*it);

        /* Check if event match the filter */
        if ((ev->type >= minType) && (ev->type <= maxType)) {
            /* Deleting the object and removing it from the list */
            delete ev;
            it = eventQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SDLEventQueue::flush(Uint32 mask)
{
    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL1::SDL_Event* ev = static_cast<SDL1::SDL_Event*>(*it);

        /* Check if event match the filter */
        if (mask & SDL1_EVENTMASK(ev->type)) {
            /* Deleting the object and removing it from the list */
            delete ev;
            it = eventQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SDLEventQueue::applyFilter(SDL_EventFilter filter, void* userdata)
{
    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL_Event* ev = static_cast<SDL_Event*>(*it);

        /* Run the filter function and check the result */
        int isKept = filter(userdata, ev);
        if (!isKept) {
            /* Deleting the object and removing it from the list */
            delete ev;
            it = eventQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SDLEventQueue::setFilter(SDL_EventFilter filter, void* userdata)
{
    filterFunc = filter;
    filterData = userdata;
}

void SDLEventQueue::setFilter(SDL1::SDL_EventFilter filter)
{
    filterFunc1 = filter;
}

bool SDLEventQueue::getFilter(SDL_EventFilter* filter, void** userdata)
{
    if (filterFunc == nullptr)
        return false;
    *filter = filterFunc;
    *userdata = filterData;
    return true;
}

SDL1::SDL_EventFilter SDLEventQueue::getFilter(void)
{
    return filterFunc1;
}

void SDLEventQueue::addWatch(SDL_EventFilter filter, void* userdata)
{
    watches.insert(std::make_pair(filter, userdata));
}

void SDLEventQueue::delWatch(SDL_EventFilter filter, void* userdata)
{
    std::set<std::pair<SDL_EventFilter,void*>>::iterator it = watches.find(std::make_pair(filter, userdata));
    if (it != watches.end())
        watches.erase(it);
}

bool SDLEventQueue::waitForEmpty()
{
    int attempts = 0;

    while (!emptied) {
        if (++attempts > 10 * 100) {
            debuglogstdio(LCF_EVENTS | LCF_SDL | LCF_ERROR | LCF_ALERT, "SDL events sync took too long, were asynchronous events incorrectly enabled?");
            return false;
        }
        struct timespec sleepTime = { 0, 10 * 1000 };
        NATIVECALL(nanosleep(&sleepTime, NULL));
    }
    return true;
}

void SDLEventQueue::resetEmpty()
{
    emptied = false;
}

}
