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

#include "EventQueue.h"
#include "logging.h"
#include "string.h"

EventQueue sdlEventQueue;

EventQueue::~EventQueue()
{
    for (auto ev: eventQueue) {
        if (SDLver == 1)
            delete (SDL1::SDL_Event*) ev;
        if (SDLver == 2)
            delete (SDL_Event*) ev;
    }
}

#define EVENTQUEUE_MAXLEN 1024

void EventQueue::insert(SDL_Event* event)
{
    if (eventQueue.size() > 1024)
        debuglog(LCF_SDL | LCF_EVENTS, "We reached the limit of the event queue size!");

    /* Building a dynamically allocated event */
    /* TODO: Hmmm... creating and destroying objects that many times
     * does not seem like a good pattern...
     */
    SDL_Event* ev = new SDL_Event;
    memcpy(ev, event, sizeof(SDL_Event));

    /* Push the event at the end of the queue */
    eventQueue.push_back((void*)ev);
}

void EventQueue::insert(SDL1::SDL_Event* event)
{
    if (eventQueue.size() > 1024)
        debuglog(LCF_SDL | LCF_EVENTS, "We reached the limit of the event queue size!");

    /* Building a dynamically allocated event */
    SDL1::SDL_Event* ev = new SDL1::SDL_Event;
    memcpy(ev, event, sizeof(SDL1::SDL_Event));

    /* Push the event at the end of the queue */
    eventQueue.push_back((void*)ev);
}

int EventQueue::pop(SDL_Event* events, int num, Uint32 minType, Uint32 maxType, bool update)
{
    int evi = 0;

    if (num <= 0)
        return 0;

    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL_Event* ev = (SDL_Event*) (*it);

        /* Check if event match the filter */
        if ((ev->type >= minType) && (ev->type <= maxType)) {
    
            /* Copy the event in the array */
            memcpy(&events[evi], ev, sizeof(SDL_Event));
            evi++;

            if (update) {
                /* Deleting the object and removing it from the list */
                delete (SDL_Event*)(*it);
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

    return evi;

}

int EventQueue::pop(SDL1::SDL_Event* events, int num, Uint32 mask, bool update)
{
    int evi = 0;

    if (num <= 0)
        return 0;

    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL1::SDL_Event* ev = (SDL1::SDL_Event*) (*it);

        /* Check if event match the filter */
        if (mask & SDL_EVENTMASK(ev->type)) {
    
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

    return evi;

}

void EventQueue::flush(Uint32 minType, Uint32 maxType)
{
    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL_Event* ev = (SDL_Event*) (*it);

        /* Check if event match the filter */
        if ((ev->type >= minType) && (ev->type <= maxType)) {
            /* Deleting the object and removing it from the list */
            delete (SDL_Event*)(*it);
            it = eventQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}

void EventQueue::flush(Uint32 mask)
{
    std::list<void*>::iterator it = eventQueue.begin();
    while (it != eventQueue.end()) {
        SDL1::SDL_Event* ev = (SDL1::SDL_Event*) (*it);

        /* Check if event match the filter */
        if (mask & SDL_EVENTMASK(ev->type)) {
            /* Deleting the object and removing it from the list */
            delete (SDL1::SDL_Event*)(*it);
            it = eventQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}


