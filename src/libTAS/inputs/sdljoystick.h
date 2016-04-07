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

#ifndef LIBTAS_SDLJOYSTICK_H_INCL
#define LIBTAS_SDLJOYSTICK_H_INCL

#include "../../external/SDL.h"
#include "../global.h"

typedef int SDL_Joystick;

/* A structure that encodes the stable unique id for a joystick device */
typedef struct {
    Uint8 data[16];
} SDL_JoystickGUID;

/**
 *  Count the number of joysticks attached to the system right now
 */
OVERRIDE int SDL_NumJoysticks(void);

/**
 *  Return the GUID for this opened joystick
 */
OVERRIDE SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick);

/**
 *  Return the name for this currently opened joystick.
 *  If no name can be found, this function returns NULL.
 */
OVERRIDE const char *SDL_JoystickName(SDL_Joystick * joystick);

/**
 *  Get the number of general axis controls on a joystick.
 */
OVERRIDE int SDL_JoystickNumAxes(SDL_Joystick * joystick);

/**
 *  Get the number of trackballs on a joystick.
 *
 *  Joystick trackballs have only relative motion events associated
 *  with them and their state cannot be polled.
 */
OVERRIDE int SDL_JoystickNumBalls(SDL_Joystick * joystick);

/**
 *  Get the number of POV hats on a joystick.
 */
OVERRIDE int SDL_JoystickNumHats(SDL_Joystick * joystick);

/**
 *  Get the number of buttons on a joystick.
 */
OVERRIDE int SDL_JoystickNumButtons(SDL_Joystick * joystick);

#endif

