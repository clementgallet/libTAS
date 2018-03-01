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

// #include "../../external/SDL.h"
#include <SDL2/SDL.h>
#include "../global.h"

namespace libtas {

/**
 *  Count the number of joysticks attached to the system right now
 */
OVERRIDE int SDL_NumJoysticks(void);

/**
 *  Get the implementation dependent name of a joystick.
 *  This can be called before any joysticks are opened.
 *  If no name can be found, this function returns NULL.
 */
OVERRIDE const char *SDL_JoystickNameForIndex(int device_index);

/**
 *  Open a joystick for use.
 *  The index passed as an argument refers to the N'th joystick on the system.
 *  This index is not the value which will identify this joystick in future
 *  joystick events.  The joystick's instance id (::SDL_JoystickID) will be used
 *  there instead.
 *
 *  \return A joystick identifier, or NULL if an error occurred.
 */
OVERRIDE SDL_Joystick *SDL_JoystickOpen(int device_index);

/**
 * Return the SDL_Joystick associated with an instance id.
 */
OVERRIDE SDL_Joystick *SDL_JoystickFromInstanceID(SDL_JoystickID joyid);

/**
 *  Get the USB vendor ID of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the vendor ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceVendor(int device_index);

/**
 *  Get the USB product ID of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the product ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceProduct(int device_index);

/**
 *  Get the product version of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the product version isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceProductVersion(int device_index);

/**
 *  Get the type of a joystick, if available.
 *  This can be called before any joysticks are opened.
 */
OVERRIDE SDL_JoystickType SDL_JoystickGetDeviceType(int device_index);

/**
 *  Return the GUID for the joystick at this index
 */
OVERRIDE SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index);

/**
 *  Get the USB vendor ID of an opened joystick, if available.
 *  If the vendor ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick);

/**
 *  Get the USB product ID of an opened joystick, if available.
 *  If the product ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick);

/**
 *  Get the product version of an opened joystick, if available.
 *  If the product version isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick);

/**
 *  Get the type of an opened joystick.
 */
OVERRIDE SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick);

/**
 *  Return the GUID for this opened joystick
 */
OVERRIDE SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick);

/**
 *  Return the name for this currently opened joystick.
 *  If no name can be found, this function returns NULL.
 *  Note: in SDL 1, there is a function of the same name,
 *  which is actually SDL_JoystickNameForIndex(int).
 */
OVERRIDE const char *SDL_JoystickName(SDL_Joystick * joystick);

/**
 *  Returns SDL_TRUE if the joystick has been opened and currently connected, or SDL_FALSE if it has not.
 */
OVERRIDE SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick);

/** SDL 1.2
 * Returns 1 if the joystick has been opened, or 0 if it has not.
 */
OVERRIDE int SDL_JoystickOpened(int device_index);

/**
 *  Get the instance ID of an opened joystick or -1 if the joystick is invalid.
 */
OVERRIDE SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick);

/** SDL 1.2
 * Get the device index of an opened joystick.
 */
OVERRIDE int SDL_JoystickIndex(SDL_Joystick *joystick);

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

/**
 *  Update the current state of the open joysticks.
 *
 *  This is called automatically by the event loop if any joystick
 *  events are enabled.
 */
OVERRIDE void SDL_JoystickUpdate(void);

/**
 *  Enable/disable joystick event polling.
 *
 *  If joystick events are disabled, you must call SDL_JoystickUpdate()
 *  yourself and check the state of the joystick when you want joystick
 *  information.
 *
 *  The state can be one of ::SDL_QUERY, ::SDL_ENABLE or ::SDL_IGNORE.
 */
OVERRIDE int SDL_JoystickEventState(int state);

/**
 *  Get the current state of an axis control on a joystick.
 *
 *  The state is a value ranging from -32768 to 32767.
 *
 *  The axis indices start at index 0.
 */
OVERRIDE Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis);

/**
 *  Get the current state of a POV hat on a joystick.
 *
 *  The hat indices start at index 0.
 *
 *  \return The return value is one of the following positions:
 *           - ::SDL_HAT_CENTERED
 *           - ::SDL_HAT_UP
 *           - ::SDL_HAT_RIGHT
 *           - ::SDL_HAT_DOWN
 *           - ::SDL_HAT_LEFT
 *           - ::SDL_HAT_RIGHTUP
 *           - ::SDL_HAT_RIGHTDOWN
 *           - ::SDL_HAT_LEFTUP
 *           - ::SDL_HAT_LEFTDOWN
 */
OVERRIDE Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick, int hat);

/**
 *  Get the ball axis change since the last poll.
 *
 *  \return 0, or -1 if you passed it invalid parameters.
 *
 *  The ball indices start at index 0.
 */
OVERRIDE int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy);

/**
 *  Get the current state of a button on a joystick.
 *
 *  The button indices start at index 0.
 */
OVERRIDE Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button);

/**
 *  Close a joystick previously opened with SDL_JoystickOpen().
 */
OVERRIDE void SDL_JoystickClose(SDL_Joystick * joystick);

/**
 *  Return the battery level of this joystick
 */
OVERRIDE SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick);

}

#endif
