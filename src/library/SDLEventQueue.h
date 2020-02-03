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

#ifndef LIBTAS_SDLEVENTQUEUE_H_INCLUDED
#define LIBTAS_SDLEVENTQUEUE_H_INCLUDED

#include <list>
#include <set>
#include <mutex>
#include "../external/SDL1.h"
#include <SDL2/SDL.h>
#include "sdlevents.h" // SDL_EventFilter

namespace libtas {
/* This is a replacement of the SDL event queue.
 * It allows us to perform our own filtering, for exemple
 * to remove the original input events.
 * It makes the implementation of SDL events API functions easier,
 * by having more control of what is inside the queue.
 * Also, it supports both SDL1 and SDL2 events.
 */
class SDLEventQueue
{
    public:
        ~SDLEventQueue();

        void init();

        /* Try to insert an event in the queue if conditions are met.
         * For SDL1 events, returns 0 if event was inserted and -1 if not.
         */
        int insert(SDL1::SDL_Event* event);

        /* For SDL2 events, returns 1 if event was inserted, 0 if event was
         * filtered and -1 if event queue is full.
         */
        int insert(SDL_Event* event);

        /* Return a number of events from the queue.
         * @param events [OUT]  array of events to write to
         * @param num     [IN]  number of events to return
         * @param mask    [IN]  mask of events that are asked
         * @param minType [IN]  lower bound of the event type
         * @param maxType [IN]  higher bound of the event type
         * @param update  [IN]  events should be removed from the queue?
         * @return              the actual number of returned events.
         */
        int pop(SDL1::SDL_Event* events, int num, Uint32 mask, bool update);
        int pop(SDL_Event* events, int num, Uint32 minType, Uint32 maxType, bool update);

        /* Flush the event queue */
        void flush(Uint32 mask);
        void flush(Uint32 minType, Uint32 maxType);

        /* Enable an event type to be inserted */
        void enable(int type);
        /* Disable an event type to be inserted */
        void disable(int type);
        /* Check if an event type is enabled */
        bool isEnabled(int type);

        /* Remove each event from the queue when
         * filter(userdata, event) returns 0.
         * Function filter has the following prototype:
         *     int filter(void* userdata, SDL_Event* event)
         */
        void applyFilter(SDL_EventFilter filter, void* userdata);

        /* Set a filter that will be called when an event is added
         * to the queue. For SDL 1.2 we don't have userdata.
         */
        void setFilter(SDL_EventFilter filter, void* userdata);
        void setFilter(SDL1::SDL_EventFilter filter);

        /* Return the filter */
        bool getFilter(SDL_EventFilter* filter, void** userdata);
        SDL1::SDL_EventFilter getFilter(void);

        /* Add a watch that is called when an event is inserted */
        void addWatch(SDL_EventFilter filter, void* userdata);

        /* Remove a watch */
        void delWatch(SDL_EventFilter filter, void* userdata);

        /* Wait for the queue to become empty */
        bool waitForEmpty();

        /* Reset the empty state of each queue */
        void resetEmpty();

        /* Mutex for protecting empied and pop() */
        std::mutex mutex;

    private:
        std::list<void*> eventQueue;
        std::set<int> droppedEvents;
        std::set<std::pair<SDL_EventFilter,void*>> watches;
        SDL1::SDL_EventFilter filterFunc1 = nullptr;
        SDL_EventFilter filterFunc = nullptr;
        void* filterData = nullptr;

        /* Was the queue emptied? Used for asynchronous events */
        bool emptied;

        /* We don't want some events to be pushed by the game */
        bool isBannedEvent(SDL_Event *event);
        bool isBannedEvent(SDL1::SDL_Event *event);
};

extern SDLEventQueue sdlEventQueue;

}

#endif
