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

#ifndef LIBTAS_INPUTS_H_INCL
#define LIBTAS_INPUTS_H_INCL

#include "../../external/SDL.h"
#include "../global.h"

extern struct AllInputs ai;
extern struct AllInputs old_ai;

/* 
 * Generate at most `num` events of type SDL_KEYUP, store them in `events`
 * and update the input structures depending on the value `update`
 * If update is true, the events won't be generated again on a future call
 */
int generateKeyUpEvent(void *events, void* gameWindow, int num, int update);

/* Same as above but with events SDL_KEYDOWN */
int generateKeyDownEvent(void *events, void* gameWindow, int num, int update);

/* Generate at most `num` events indicating that a controller was plugged in */
int generateControllerAdded(SDL_Event* events, int num, int update);

/* Same as KeyUp/KeyDown functions but with controller events */
int generateControllerEvent(SDL_Event* events, int num, int update);

#endif

