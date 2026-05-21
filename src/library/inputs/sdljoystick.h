/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include "hook.h"

namespace libtas {

/* Detach a joystick */
void mySDL_JoystickDetached(int index);

/* Report if events are enabled for a joystick */
bool mySDL_JoystickReportEvents(int index);

/**
 *  Count the number of joysticks attached to the system right now
 */
OVERRIDE int SDL_NumJoysticks(void);

/**
 * Return whether a joystick is currently connected.
 *
 * \returns true if a joystick is connected, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoysticks
 */
OVERRIDE bool SDL_HasJoystick(void);

/**
 * Get a list of currently connected joysticks.
 *
 * \param count a pointer filled in with the number of joysticks returned, may
 *              be NULL.
 * \returns a 0 terminated array of joystick instance IDs or NULL on failure;
 *          call SDL_GetError() for more information. This should be freed
 *          with SDL_free() when it is no longer needed.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_HasJoystick
 * \sa SDL_OpenJoystick
 */
OVERRIDE sdl3::SDL_JoystickID * SDL_GetJoysticks(int *count);

/**
 *  Get the implementation dependent name of a joystick.
 *  This can be called before any joysticks are opened.
 *  If no name can be found, this function returns NULL.
 */
OVERRIDE const char *SDL_JoystickNameForIndex(int device_index);

/**
 * Get the implementation dependent name of a joystick.
 *
 * This can be called before any joysticks are opened.
 *
 * \param instance_id the joystick instance ID.
 * \returns the name of the selected joystick. If no name can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickName
 * \sa SDL_GetJoysticks
 */
OVERRIDE const char * SDL_GetJoystickNameForID(sdl3::SDL_JoystickID instance_id);

/**
 * Get the implementation dependent path of a joystick.
 *
 * This can be called before any joysticks are opened.
 *
 * \param device_index the index of the joystick to query (the N'th joystick
 *                     on the system).
 * \returns the path of the selected joystick. If no path can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 2.24.0.
 *
 * \sa SDL_JoystickPath
 * \sa SDL_JoystickOpen
 */
OVERRIDE const char *SDL_JoystickPathForIndex(int device_index);

/**
 * Get the implementation dependent path of a joystick.
 *
 * This can be called before any joysticks are opened.
 *
 * \param instance_id the joystick instance ID.
 * \returns the path of the selected joystick. If no path can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickPath
 * \sa SDL_GetJoysticks
 */
OVERRIDE const char * SDL_GetJoystickPathForID(sdl3::SDL_JoystickID instance_id);

/**
 * Get the implementation dependent path of a joystick.
 *
 * \param joystick the SDL_Joystick obtained from SDL_JoystickOpen().
 * \returns the path of the selected joystick. If no path can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 2.24.0.
 *
 * \sa SDL_JoystickPathForIndex
 */
OVERRIDE const char *SDL_JoystickPath(SDL_Joystick *joystick);

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
 * Open a joystick for use.
 *
 * The joystick subsystem must be initialized before a joystick can be opened
 * for use.
 *
 * \param instance_id the joystick instance ID.
 * \returns a joystick identifier or NULL on failure; call SDL_GetError() for
 *          more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CloseJoystick
 */
OVERRIDE SDL_Joystick * SDL_OpenJoystick(sdl3::SDL_JoystickID instance_id);

/**
 * Return the SDL_Joystick associated with an instance id.
 */
OVERRIDE SDL_Joystick *SDL_JoystickFromInstanceID(sdl2::SDL_JoystickID joyid);

/**
 * Get the SDL_Joystick associated with an instance ID, if it has been opened.
 *
 * \param instance_id the instance ID to get the SDL_Joystick for.
 * \returns an SDL_Joystick on success or NULL on failure or if it hasn't been
 *          opened yet; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE SDL_Joystick * SDL_GetJoystickFromID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the USB vendor ID of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the vendor ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceVendor(int device_index);

/**
 * Get the USB vendor ID of a joystick, if available.
 *
 * This can be called before any joysticks are opened. If the vendor ID isn't
 * available this function returns 0.
 *
 * \param instance_id the joystick instance ID.
 * \returns the USB vendor ID of the selected joystick. If called with an
 *          invalid instance_id, this function returns 0.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickVendor
 * \sa SDL_GetJoysticks
 */
OVERRIDE Uint16 SDL_GetJoystickVendorForID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the USB product ID of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the product ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceProduct(int device_index);

/**
 * Get the USB product ID of a joystick, if available.
 *
 * This can be called before any joysticks are opened. If the product ID isn't
 * available this function returns 0.
 *
 * \param instance_id the joystick instance ID.
 * \returns the USB product ID of the selected joystick. If called with an
 *          invalid instance_id, this function returns 0.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickProduct
 * \sa SDL_GetJoysticks
 */
OVERRIDE Uint16 SDL_GetJoystickProductForID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the product version of a joystick, if available.
 *  This can be called before any joysticks are opened.
 *  If the product version isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetDeviceProductVersion(int device_index);

/**
 * Get the product version of a joystick, if available.
 *
 * This can be called before any joysticks are opened. If the product version
 * isn't available this function returns 0.
 *
 * \param instance_id the joystick instance ID.
 * \returns the product version of the selected joystick. If called with an
 *          invalid instance_id, this function returns 0.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickProductVersion
 * \sa SDL_GetJoysticks
 */
OVERRIDE Uint16 SDL_GetJoystickProductVersionForID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the type of a joystick, if available.
 *  This can be called before any joysticks are opened.
 */
OVERRIDE sdl2::SDL_JoystickType SDL_JoystickGetDeviceType(int device_index);

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
OVERRIDE sdl2::SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index);

/**
 *  Return the GUID for the joystick at this index
 */
OVERRIDE sdl2::SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index);

/**
 * Get the implementation-dependent GUID of a joystick.
 *
 * This can be called before any joysticks are opened.
 *
 * \param instance_id the joystick instance ID.
 * \returns the GUID of the selected joystick. If called with an invalid
 *          instance_id, this function returns a zero GUID.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickGUID
 * \sa SDL_GUIDToString
 */
OVERRIDE SDL_GUID SDL_GetJoystickGUIDForID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the USB vendor ID of an opened joystick, if available.
 *  If the vendor ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick);

/**
 * Get the USB vendor ID of an opened joystick, if available.
 *
 * If the vendor ID isn't available this function returns 0.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the USB vendor ID of the selected joystick, or 0 if unavailable.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickVendorForID
 */
OVERRIDE Uint16 SDL_GetJoystickVendor(SDL_Joystick *joystick);

/**
 *  Get the USB product ID of an opened joystick, if available.
 *  If the product ID isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick);

/**
 * Get the USB product ID of an opened joystick, if available.
 *
 * If the product ID isn't available this function returns 0.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the USB product ID of the selected joystick, or 0 if unavailable.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickProductForID
 */
OVERRIDE Uint16 SDL_GetJoystickProduct(SDL_Joystick *joystick);

/**
 *  Get the product version of an opened joystick, if available.
 *  If the product version isn't available this function returns 0.
 */
OVERRIDE Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick);

/**
 * Get the product version of an opened joystick, if available.
 *
 * If the product version isn't available this function returns 0.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the product version of the selected joystick, or 0 if unavailable.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickProductVersionForID
 */
OVERRIDE Uint16 SDL_GetJoystickProductVersion(SDL_Joystick *joystick);

/**
 *  Get the type of an opened joystick.
 */
OVERRIDE sdl2::SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick);

/**
 * Get the type of an opened joystick.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the SDL_JoystickType of the selected joystick.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickTypeForID
 */
OVERRIDE sdl3::SDL_JoystickType SDL_GetJoystickType(SDL_Joystick *joystick);

/**
 *  Return the GUID for this opened joystick
 */
OVERRIDE sdl2::SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick);

/**
 * Get the implementation-dependent GUID for the joystick.
 *
 * This function requires an open joystick.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the GUID of the given joystick. If called on an invalid index,
 *          this function returns a zero GUID; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickGUIDForID
 * \sa SDL_GUIDToString
 */

OVERRIDE SDL_GUID SDL_GetJoystickGUID(SDL_Joystick *joystick);

/**
 *  Return the name for this currently opened joystick.
 *  If no name can be found, this function returns NULL.
 *  Note: in SDL 1, there is a function of the same name,
 *  which is actually SDL_JoystickNameForIndex(int).
 */
OVERRIDE const char *SDL_JoystickName(SDL_Joystick * joystick);

/**
 * Get the implementation dependent name of a joystick.
 *
 * \param joystick the SDL_Joystick obtained from SDL_OpenJoystick().
 * \returns the name of the selected joystick. If no name can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickNameForID
 */
OVERRIDE const char * SDL_GetJoystickName(SDL_Joystick *joystick);

/**
 *  Returns SDL_TRUE if the joystick has been opened and currently connected, or SDL_FALSE if it has not.
 */
OVERRIDE SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick);

/**
 * Get the status of a specified joystick.
 *
 * \param joystick the joystick to query.
 * \returns true if the joystick has been opened, false if it has not; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_JoystickConnected(SDL_Joystick *joystick);

/** SDL 1.2
 * Returns 1 if the joystick has been opened, or 0 if it has not.
 */
OVERRIDE int SDL_JoystickOpened(int device_index);

/**
 *  Get the instance ID of an opened joystick or -1 if the joystick is invalid.
 */
OVERRIDE sdl2::SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick);

/**
 * Get the instance ID of an opened joystick.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \returns the instance ID of the specified joystick on success or 0 on
 *          failure; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE sdl3::SDL_JoystickID SDL_GetJoystickID(SDL_Joystick *joystick);

/** SDL 1.2
 * Get the device index of an opened joystick.
 */
OVERRIDE int SDL_JoystickIndex(SDL_Joystick *joystick);

/**
 *  Get the number of general axis controls on a joystick.
 */
OVERRIDE int SDL_JoystickNumAxes(SDL_Joystick * joystick);

/**
 * Get the number of general axis controls on a joystick.
 *
 * Often, the directional pad on a game controller will either look like 4
 * separate buttons or a POV hat, and not axes, but all of this is up to the
 * device and platform.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \returns the number of axis controls/number of axes on success or -1 on
 *          failure; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickAxis
 * \sa SDL_GetNumJoystickBalls
 * \sa SDL_GetNumJoystickButtons
 * \sa SDL_GetNumJoystickHats
 */
OVERRIDE int SDL_GetNumJoystickAxes(SDL_Joystick *joystick);

/**
 *  Get the number of trackballs on a joystick.
 *
 *  Joystick trackballs have only relative motion events associated
 *  with them and their state cannot be polled.
 */
OVERRIDE int SDL_JoystickNumBalls(SDL_Joystick * joystick);

/**
 * Get the number of trackballs on a joystick.
 *
 * Joystick trackballs have only relative motion events associated with them
 * and their state cannot be polled.
 *
 * Most joysticks do not have trackballs.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \returns the number of trackballs on success or -1 on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickBall
 * \sa SDL_GetNumJoystickAxes
 * \sa SDL_GetNumJoystickButtons
 * \sa SDL_GetNumJoystickHats
 */
OVERRIDE int SDL_GetNumJoystickBalls(SDL_Joystick *joystick);

/**
 *  Get the number of POV hats on a joystick.
 */
OVERRIDE int SDL_JoystickNumHats(SDL_Joystick * joystick);

/**
 * Get the number of POV hats on a joystick.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \returns the number of POV hats on success or -1 on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickHat
 * \sa SDL_GetNumJoystickAxes
 * \sa SDL_GetNumJoystickBalls
 * \sa SDL_GetNumJoystickButtons
 */
OVERRIDE int SDL_GetNumJoystickHats(SDL_Joystick *joystick);

/**
 *  Get the number of buttons on a joystick.
 */
OVERRIDE int SDL_JoystickNumButtons(SDL_Joystick * joystick);

/**
 * Get the number of buttons on a joystick.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \returns the number of buttons on success or -1 on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoystickButton
 * \sa SDL_GetNumJoystickAxes
 * \sa SDL_GetNumJoystickBalls
 * \sa SDL_GetNumJoystickHats
 */
OVERRIDE int SDL_GetNumJoystickButtons(SDL_Joystick *joystick);

/**
 *  Update the current state of the open joysticks.
 *
 *  This is called automatically by the event loop if any joystick
 *  events are enabled.
 */
OVERRIDE void SDL_JoystickUpdate(void);

/**
 * Update the current state of the open joysticks.
 *
 * This is called automatically by the event loop if any joystick events are
 * enabled.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE void SDL_UpdateJoysticks(void);

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
 * Set the state of joystick event processing.
 *
 * If joystick events are disabled, you must call SDL_UpdateJoysticks()
 * yourself and check the state of the joystick when you want joystick
 * information.
 *
 * \param enabled whether to process joystick events or not.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_JoystickEventsEnabled
 * \sa SDL_UpdateJoysticks
 */
OVERRIDE void SDL_SetJoystickEventsEnabled(bool enabled);

/**
 * Query the state of joystick event processing.
 *
 * If joystick events are disabled, you must call SDL_UpdateJoysticks()
 * yourself and check the state of the joystick when you want joystick
 * information.
 *
 * \returns true if joystick events are being processed, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetJoystickEventsEnabled
 */
OVERRIDE bool SDL_JoystickEventsEnabled(void);

/**
 *  Get the current state of an axis control on a joystick.
 *
 *  The state is a value ranging from -32768 to 32767.
 *
 *  The axis indices start at index 0.
 */
OVERRIDE Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis);

/**
 * Get the current state of an axis control on a joystick.
 *
 * SDL makes no promises about what part of the joystick any given axis refers
 * to. Your game should have some sort of configuration UI to let users
 * specify what each axis should be bound to. Alternately, SDL's higher-level
 * Game Controller API makes a great effort to apply order to this lower-level
 * interface, so you know that a specific axis is the "left thumb stick," etc.
 *
 * The value returned by SDL_GetJoystickAxis() is a signed integer (-32768 to
 * 32767) representing the current position of the axis. It may be necessary
 * to impose certain tolerances on these values to account for jitter.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \param axis the axis to query; the axis indices start at index 0.
 * \returns a 16-bit signed integer representing the current position of the
 *          axis or 0 on failure; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetNumJoystickAxes
 */
OVERRIDE Sint16 SDL_GetJoystickAxis(SDL_Joystick *joystick, int axis);

/**
 * Get the initial state of an axis control on a joystick.
 *
 * The state is a value ranging from -32768 to 32767.
 *
 * The axis indices start at index 0.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \param axis the axis to query; the axis indices start at index 0.
 * \param state upon return, the initial value is supplied here.
 * \returns true if this axis has any initial value, or false if not.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_GetJoystickAxisInitialState(SDL_Joystick *joystick, int axis, Sint16 *state);

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
 * Get the current state of a POV hat on a joystick.
 *
 * The returned value will be one of the `SDL_HAT_*` values.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \param hat the hat index to get the state from; indices start at index 0.
 * \returns the current hat position.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetNumJoystickHats
 */
OVERRIDE Uint8 SDL_GetJoystickHat(SDL_Joystick *joystick, int hat);

/**
 *  Get the ball axis change since the last poll.
 *
 *  \return 0, or -1 if you passed it invalid parameters.
 *
 *  The ball indices start at index 0.
 */
OVERRIDE int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy);

/**
 * Get the ball axis change since the last poll.
 *
 * Trackballs can only return relative motion since the last call to
 * SDL_GetJoystickBall(), these motion deltas are placed into `dx` and `dy`.
 *
 * Most joysticks do not have trackballs.
 *
 * \param joystick the SDL_Joystick to query.
 * \param ball the ball index to query; ball indices start at index 0.
 * \param dx stores the difference in the x axis position since the last poll.
 * \param dy stores the difference in the y axis position since the last poll.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetNumJoystickBalls
 */
OVERRIDE bool SDL_GetJoystickBall(SDL_Joystick *joystick, int ball, int *dx, int *dy);

/**
 *  Get the current state of a button on a joystick.
 *
 *  The button indices start at index 0.
 */
OVERRIDE Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button);

/**
 * Get the current state of a button on a joystick.
 *
 * \param joystick an SDL_Joystick structure containing joystick information.
 * \param button the button index to get the state from; indices start at
 *               index 0.
 * \returns true if the button is pressed, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetNumJoystickButtons
 */
OVERRIDE bool SDL_GetJoystickButton(SDL_Joystick *joystick, int button);

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
 * Start a rumble effect.
 *
 * Each call to this function cancels any previous rumble effect, and calling
 * it with 0 intensity stops any rumbling.
 *
 * This function requires you to process SDL events or call
 * SDL_UpdateJoysticks() to update rumble state.
 *
 * \param joystick the joystick to vibrate.
 * \param low_frequency_rumble the intensity of the low frequency (left)
 *                             rumble motor, from 0 to 0xFFFF.
 * \param high_frequency_rumble the intensity of the high frequency (right)
 *                              rumble motor, from 0 to 0xFFFF.
 * \param duration_ms the duration of the rumble effect, in milliseconds.
 * \returns true, or false if rumble isn't supported on this joystick.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDLCALL SDL_RumbleJoystick(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

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
 * Start a rumble effect in the joystick's triggers.
 *
 * Each call to this function cancels any previous trigger rumble effect, and
 * calling it with 0 intensity stops any rumbling.
 *
 * Note that this is rumbling of the _triggers_ and not the game controller as
 * a whole. This is currently only supported on Xbox One controllers. If you
 * want the (more common) whole-controller rumble, use SDL_RumbleJoystick()
 * instead.
 *
 * This function requires you to process SDL events or call
 * SDL_UpdateJoysticks() to update rumble state.
 *
 * \param joystick the joystick to vibrate.
 * \param left_rumble the intensity of the left trigger rumble motor, from 0
 *                    to 0xFFFF.
 * \param right_rumble the intensity of the right trigger rumble motor, from 0
 *                     to 0xFFFF.
 * \param duration_ms the duration of the rumble effect, in milliseconds.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_RumbleJoystick
 */
OVERRIDE bool SDL_RumbleJoystickTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);

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
 * Update a joystick's LED color.
 *
 * An example of a joystick LED is the light on the back of a PlayStation 4's
 * DualShock 4 controller.
 *
 * For joysticks with a single color LED, the maximum of the RGB values will
 * be used as the LED brightness.
 *
 * \param joystick the joystick to update.
 * \param red the intensity of the red LED.
 * \param green the intensity of the green LED.
 * \param blue the intensity of the blue LED.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_SetJoystickLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue);

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
 * Send a joystick specific effect packet.
 *
 * \param joystick the joystick to affect.
 * \param data the data to send to the joystick.
 * \param size the size of the data to send to the joystick.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_SendJoystickEffect(SDL_Joystick *joystick, const void *data, int size);

/**
 *  Close a joystick previously opened with SDL_JoystickOpen().
 */
OVERRIDE void SDL_JoystickClose(SDL_Joystick * joystick);

/**
 * Close a joystick previously opened with SDL_OpenJoystick().
 *
 * \param joystick the joystick device to close.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_OpenJoystick
 */
OVERRIDE void SDL_CloseJoystick(SDL_Joystick *joystick);

/**
 *  Return the battery level of this joystick
 */
OVERRIDE sdl2::SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick);

/**
 * Get the battery state of a joystick.
 *
 * You should never take a battery status as absolute truth. Batteries
 * (especially failing batteries) are delicate hardware, and the values
 * reported here are best estimates based on what that hardware reports. It's
 * not uncommon for older batteries to lose stored power much faster than it
 * reports, or completely drain when reporting it has 20 percent left, etc.
 *
 * \param joystick the joystick to query.
 * \param percent a pointer filled in with the percentage of battery life
 *                left, between 0 and 100, or NULL to ignore. This will be
 *                filled in with -1 we can't determine a value or there is no
 *                battery.
 * \returns the current battery state or `SDL_POWERSTATE_ERROR` on failure;
 *          call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE sdl3::SDL_PowerState SDL_GetJoystickPowerInfo(SDL_Joystick *joystick, int *percent);

}

#endif
