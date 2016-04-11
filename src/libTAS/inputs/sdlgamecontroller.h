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

#ifndef LIBTAS_SDLGAMECONTROLLER_H_INCL
#define LIBTAS_SDLGAMECONTROLLER_H_INCL

#include "sdljoystick.h"
#include "../../external/SDL.h"
#include "inputs.h"
#include "../global.h"

/* 
 * Declaring SDL_GameController structs
 * to be simply an int containing the controller id.
 * This struct is normally internal struct that is not
 * revealed by the API, so members of the struct won't be used
 * by the games.
 * The downside is that we have to implement all functions that
 * use this struct, otherwise we will probably get a crash.
 */
typedef int SDL_GameController;

/**
 *  Is the joystick on this index supported by the game controller interface?
 */
OVERRIDE SDL_bool SDL_IsGameController(int joystick_index);

/**
 *  Open a game controller for use.
 *  The index passed as an argument refers to the N'th game controller on the system.
 *  This index is the value which will identify this controller in future controller
 *  events.
 *
 *  \return A controller identifier, or NULL if an error occurred.
 */
OVERRIDE SDL_GameController *SDL_GameControllerOpen(int joystick_index);

/**
 *  Get the underlying joystick object used by a controller
 */
OVERRIDE SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController* gamecontroller);

/**
 * Return the SDL_GameController associated with an instance id.
 */
OVERRIDE SDL_GameController* SDL_GameControllerFromInstanceID(SDL_JoystickID joyid);

/**
 *  Get the implementation dependent name of a game controller.
 *  This can be called before any controllers are opened.
 *  If no name can be found, this function returns NULL.
 */
OVERRIDE const char *SDL_GameControllerNameForIndex(int joystick_index);

/**
 *  Return the name for this currently opened controller
 */
OVERRIDE const char *SDL_GameControllerName(SDL_GameController *gamecontroller);

/**
 *  Returns SDL_TRUE if the controller has been opened and currently connected,
 *  or SDL_FALSE if it has not.
 */
OVERRIDE SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);

/**
 *  Enable/disable controller event polling.
 *
 *  If controller events are disabled, you must call SDL_GameControllerUpdate()
 *  yourself and check the state of the controller when you want controller
 *  information.
 *
 *  The state can be one of ::SDL_QUERY, ::SDL_ENABLE or ::SDL_IGNORE.
 */
OVERRIDE int SDL_GameControllerEventState(int state);

/**
 *  Update the current state of the open game controllers.
 *
 *  This is called automatically by the event loop if any game controller
 *  events are enabled.
 */
OVERRIDE void SDL_GameControllerUpdate(void);

/**
 *  Get the current state of an axis control on a game controller.
 *
 *  The state is a value ranging from -32768 to 32767.
 *
 *  The axis indices start at index 0.
 */
OVERRIDE Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                                SDL_GameControllerAxis axis);

/**
 *  Get the current state of a button on a game controller.
 *
 *  The button indices start at index 0.
 */
OVERRIDE Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button);

/**
 *  Close a controller previously opened with SDL_GameControllerOpen().
 */
OVERRIDE void SDL_GameControllerClose(SDL_GameController *gamecontroller);

/* We don't need to hook these functions */
// SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis);
// SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton button);

#endif

