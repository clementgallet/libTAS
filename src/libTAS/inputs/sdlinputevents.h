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

#ifndef LIBTAS_SDLINPUTEVENTS_H_INCL
#define LIBTAS_SDLINPUTEVENTS_H_INCL

#include "../global.h"

/* Generate events of type SDL_KEYUP, store them in our emulated event queue */
void generateSDLKeyUpEvents(void);

/* Same as above but with events SDL_KEYDOWN */
void generateSDLKeyDownEvents(void);

/* Generate events indicating that a controller was plugged in */
void generateSDLControllerAdded(void);

/* Same as KeyUp/KeyDown functions but with controller events */
void generateSDLControllerEvents(void);

/* Same as above with MouseMotion event */
void generateSDLMouseMotionEvents(void);

/* Same as above with the MouseButton event */
void generateSDLMouseButtonEvents(void);

#endif
