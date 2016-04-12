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

#ifndef LIBTAS_EVENTQUEUE_H_INCLUDED
#define LIBTAS_EVENTQUEUE_H_INCLUDED

#include <list>
#include <set>
#include "../external/SDL.h"
#include "events.h" // SDL_EventFilter

class EventQueue
{
    public:
        ~EventQueue();

        void init();

        /* Try to insert an event in the queue if conditions are met */
        void insert(SDL1::SDL_Event* event);
        void insert(SDL_Event* event);

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

        /* Add a watcher that is called when an event is inserted */
        void addWatch(SDL_EventFilter filter, void* userdata);

        /* Remove a watcher */
        void delWatch(SDL_EventFilter filter, void* userdata);

    private:
        std::list<void*> eventQueue;
        std::set<int> droppedEvents;
        std::set<std::pair<SDL_EventFilter,void*>> watches;
        SDL1::SDL_EventFilter filterFunc1 = nullptr;
        SDL_EventFilter filterFunc = nullptr;
        void* filterData = nullptr;
};

extern EventQueue sdlEventQueue;

#endif

