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

class EventQueue
{
    public:
        ~EventQueue();

        void init();

        void insert(SDL1::SDL_Event* event);
        void insert(SDL_Event* event);

        int pop(SDL1::SDL_Event* events, int num, Uint32 mask, bool update);
        int pop(SDL_Event* events, int num, Uint32 minType, Uint32 maxType, bool update);

        void flush(Uint32 mask);
        void flush(Uint32 minType, Uint32 maxType);

        void enable(int type);
        void disable(int type);
        bool isEnabled(int type);

    private:
        std::list<void*> eventQueue;
        std::set<int> droppedEvents;

};

extern EventQueue sdlEventQueue;

#endif

