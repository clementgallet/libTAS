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

#ifndef LIBTAS_SDLJOYSTICK_H_INCL
#define LIBTAS_SDLJOYSTICK_H_INCL

#include <SDL2/SDL.h>
#include "../hook.h"

namespace libtas {

#if SDL_VERSION_ATLEAST(2, 0, 6)
#else
/* Adding the SDL_JoystickType enum because it was introduced in 2.0.6 and many
 * systems still use 2.0.5
 */
typedef enum
{
    SDL_JOYSTICK_TYPE_UNKNOWN,
    SDL_JOYSTICK_TYPE_GAMECONTROLLER,
    SDL_JOYSTICK_TYPE_WHEEL,
    SDL_JOYSTICK_TYPE_ARCADE_STICK,
    SDL_JOYSTICK_TYPE_FLIGHT_STICK,
    SDL_JOYSTICK_TYPE_DANCE_PAD,
    SDL_JOYSTICK_TYPE_GUITAR,
    SDL_JOYSTICK_TYPE_DRUM_KIT,
    SDL_JOYSTICK_TYPE_ARCADE_PAD,
    SDL_JOYSTICK_TYPE_THROTTLE
} SDL_JoystickType;
#endif

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
 * Get the instance ID of a joystick.
 *
 * This can be called before any joysticks are opened. If the index is out of
 * range, this function will return -1.
 *
 * \param device_index the index of the joystick to query (the N'th joystick
 *                     on the system
 * \returns the instance id of the selected joystick. If called on an invalid
 *          index, this function returns zero
 *
 * \since This function is available since SDL 2.0.6.
 */
OVERRIDE SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index);

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
 *  Trigger a rumble effect
 *  Each call to this function cancels any previous rumble effect, and calling it with 0 intensity stops any rumbling.
 *
 *  \param joystick The joystick to vibrate
 *  \param low_frequency_rumble The intensity of the low frequency (left) rumble motor, from 0 to 0xFFFF
 *  \param high_frequency_rumble The intensity of the high frequency (right) rumble motor, from 0 to 0xFFFF
 *  \param duration_ms The duration of the rumble effect, in milliseconds
 *
 *  \return 0, or -1 if rumble isn't supported on this joystick
 */
OVERRIDE int SDL_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

/**
 * Start a rumble effect in the joystick's triggers
 *
 * Each call to this function cancels any previous trigger rumble effect, and
 * calling it with 0 intensity stops any rumbling.
 *
 * Note that this is rumbling of the _triggers_ and not the game controller as
 * a whole. This is currently only supported on Xbox One controllers. If you
 * want the (more common) whole-controller rumble, use SDL_JoystickRumble()
 * instead.
 *
 * \param joystick The joystick to vibrate
 * \param left_rumble The intensity of the left trigger rumble motor, from 0
 *                    to 0xFFFF
 * \param right_rumble The intensity of the right trigger rumble motor, from 0
 *                     to 0xFFFF
 * \param duration_ms The duration of the rumble effect, in milliseconds
 * \returns 0, or -1 if trigger rumble isn't supported on this joystick
 *
 * \since This function is available since SDL 2.0.14.
 *
 * \sa SDL_JoystickHasRumbleTriggers
 */
OVERRIDE int SDL_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);

/**
 * Query whether a joystick has an LED.
 *
 * An example of a joystick LED is the light on the back of a PlayStation 4's
 * DualShock 4 controller.
 *
 * \param joystick The joystick to query
 * \return SDL_TRUE if the joystick has a modifiable LED, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_JoystickHasLED(SDL_Joystick *joystick);

/**
 * Query whether a joystick has rumble support.
 *
 * \param joystick The joystick to query
 * \return SDL_TRUE if the joystick has rumble, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.18.
 *
 * \sa SDL_JoystickRumble
 */
OVERRIDE SDL_bool SDL_JoystickHasRumble(SDL_Joystick *joystick);

/**
 * Query whether a joystick has rumble support on triggers.
 *
 * \param joystick The joystick to query
 * \return SDL_TRUE if the joystick has trigger rumble, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.18.
 *
 * \sa SDL_JoystickRumbleTriggers
 */
OVERRIDE SDL_bool SDL_JoystickHasRumbleTriggers(SDL_Joystick *joystick);

/**
 * Update a joystick's LED color.
 *
 * An example of a joystick LED is the light on the back of a PlayStation 4's
 * DualShock 4 controller.
 *
 * \param joystick The joystick to update
 * \param red The intensity of the red LED
 * \param green The intensity of the green LED
 * \param blue The intensity of the blue LED
 * \returns 0 on success, -1 if this joystick does not have a modifiable LED
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE int SDL_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue);

/**
 * Send a joystick specific effect packet
 *
 * \param joystick The joystick to affect
 * \param data The data to send to the joystick
 * \param size The size of the data to send to the joystick
 * \returns 0, or -1 if this joystick or driver doesn't support effect packets
 *
 * \since This function is available since SDL 2.0.16.
 */
OVERRIDE int SDL_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size);

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
