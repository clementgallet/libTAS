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

#ifndef LIBTAS_SDLGAMECONTROLLER_H_INCL
#define LIBTAS_SDLGAMECONTROLLER_H_INCL

#include "sdljoystick.h"

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

namespace libtas {

/* Own methods to gather or change the attached state */
bool mySDL_GameControllerIsAttached(int index);

void mySDL_GameControllerChangeAttached(int index);

/* Report if events are enabled for a game controller */
bool mySDL_GameControllerReportEvents(int index);

/**
 * Return whether a gamepad is currently connected.
 *
 * \returns true if a gamepad is connected, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGamepads
 */
OVERRIDE bool SDL_HasGamepad(void);

/**
 * Get a list of currently connected gamepads.
 *
 * \param count a pointer filled in with the number of gamepads returned, may
 *              be NULL.
 * \returns a 0 terminated array of joystick instance IDs or NULL on failure;
 *          call SDL_GetError() for more information. This should be freed
 *          with SDL_free() when it is no longer needed.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_HasGamepad
 * \sa SDL_OpenGamepad
 */
OVERRIDE sdl3::SDL_JoystickID * SDL_GetGamepads(int *count);

/**
 *  Is the joystick on this index supported by the game controller interface?
 */
OVERRIDE SDL_bool SDL_IsGameController(int joystick_index);

/**
 * Check if the given joystick is supported by the gamepad interface.
 *
 * \param instance_id the joystick instance ID.
 * \returns true if the given joystick is supported by the gamepad interface,
 *          false if it isn't or it's an invalid index.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetJoysticks
 * \sa SDL_OpenGamepad
 */
OVERRIDE bool SDL_IsGamepad(sdl3::SDL_JoystickID instance_id);

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
 * Open a gamepad for use.
 *
 * \param instance_id the joystick instance ID.
 * \returns a gamepad identifier or NULL if an error occurred; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CloseGamepad
 * \sa SDL_IsGamepad
 */
OVERRIDE SDL_Gamepad * SDL_OpenGamepad(sdl3::SDL_JoystickID instance_id);

/**
 *  Get the underlying joystick object used by a controller
 */
OVERRIDE SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController* gamecontroller);

/**
 * Get the underlying joystick from a gamepad.
 *
 * This function will give you a SDL_Joystick object, which allows you to use
 * the SDL_Joystick functions with a SDL_Gamepad object. This would be useful
 * for getting a joystick's position at any given time, even if it hasn't
 * moved (moving it would produce an event, which would have the axis' value).
 *
 * The pointer returned is owned by the SDL_Gamepad. You should not call
 * SDL_CloseJoystick() on it, for example, since doing so will likely cause
 * SDL to crash.
 *
 * \param gamepad the gamepad object that you want to get a joystick from.
 * \returns an SDL_Joystick object, or NULL on failure; call SDL_GetError()
 *          for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE SDL_Joystick * SDL_GetGamepadJoystick(SDL_Gamepad *gamepad);

/**
 * Return the SDL_GameController associated with an instance id.
 */
OVERRIDE SDL_GameController* SDL_GameControllerFromInstanceID(sdl2::SDL_JoystickID joyid);

/**
 * Get the SDL_Gamepad associated with a joystick instance ID, if it has been
 * opened.
 *
 * \param instance_id the joystick instance ID of the gamepad.
 * \returns an SDL_Gamepad on success or NULL on failure or if it hasn't been
 *          opened yet; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE SDL_Gamepad * SDL_GetGamepadFromID(sdl3::SDL_JoystickID instance_id);

/**
 *  Get a mapping string for a GUID
 *
 *  \return the mapping string.  Must be freed with SDL_free.  Returns NULL if no mapping is available
 */
OVERRIDE char *SDL_GameControllerMappingForGUID( sdl2::SDL_JoystickGUID guid );

/**
 *  Get a mapping string for an open GameController
 *
 *  \return the mapping string.  Must be freed with SDL_free.  Returns NULL if no mapping is available
 */
OVERRIDE char *SDL_GameControllerMapping( SDL_GameController * gamecontroller );

/**
 *  Get the implementation dependent name of a game controller.
 *  This can be called before any controllers are opened.
 *  If no name can be found, this function returns NULL.
 */
OVERRIDE const char *SDL_GameControllerNameForIndex(int joystick_index);

/**
 * Get the implementation dependent name of a gamepad.
 *
 * This can be called before any gamepads are opened.
 *
 * \param instance_id the joystick instance ID.
 * \returns the name of the selected gamepad. If no name can be found, this
 *          function returns NULL; call SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGamepadName
 * \sa SDL_GetGamepads
 */
OVERRIDE const char * SDL_GetGamepadNameForID(sdl3::SDL_JoystickID instance_id);

/**
 * Get the mapping of a game controller.
 *
 * This can be called before any controllers are opened.
 *
 * \param joystick_index the device_index of a device, from zero to
 *                       SDL_NumJoysticks()-1
 * \returns the mapping string. Must be freed with SDL_free(). Returns NULL if
 *          no mapping is available.
 *
 * \since This function is available since SDL 2.0.9.
 */
OVERRIDE char *SDL_GameControllerMappingForDeviceIndex(int joystick_index);

/**
 * Get the type of a game controller.
 *
 * This can be called before any controllers are opened.
 *
 * \param joystick_index the device_index of a device, from zero to
 *                       SDL_NumJoysticks()-1
 * \returns the controller type.
 *
 * \since This function is available since SDL 2.0.12.
 */
OVERRIDE sdl2::SDL_GameControllerType SDL_GameControllerTypeForIndex(int joystick_index);

/**
 *  Return the name for this currently opened controller
 */
OVERRIDE const char *SDL_GameControllerName(SDL_GameController *gamecontroller);

/**
 * Get the implementation-dependent name for an opened gamepad.
 *
 * \param gamepad a gamepad identifier previously returned by
 *                SDL_OpenGamepad().
 * \returns the implementation dependent name for the gamepad, or NULL if
 *          there is no name or the identifier passed is invalid.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGamepadNameForID
 */
OVERRIDE const char * SDL_GetGamepadName(SDL_Gamepad *gamepad);

/**
 * Get the type of this currently opened controller
 *
 * This is the same name as returned by SDL_GameControllerTypeForIndex(), but
 * it takes a controller identifier instead of the (unstable) device index.
 *
 * \param gamecontroller the game controller object to query.
 * \returns the controller type.
 *
 * \since This function is available since SDL 2.0.12.
 */
OVERRIDE sdl2::SDL_GameControllerType SDL_GameControllerGetType(SDL_GameController *gamecontroller);

/**
 * Get the type of an opened gamepad.
 *
 * \param gamepad the gamepad object to query.
 * \returns the gamepad type, or SDL_GAMEPAD_TYPE_UNKNOWN if it's not
 *          available.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGamepadTypeForID
 */
OVERRIDE sdl3::SDL_GamepadType SDL_GetGamepadType(SDL_Gamepad *gamepad);

/**
 *  Returns SDL_TRUE if the controller has been opened and currently connected,
 *  or SDL_FALSE if it has not.
 */
OVERRIDE SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);

/**
 * Check if a gamepad has been opened and is currently connected.
 *
 * \param gamepad a gamepad identifier previously returned by
 *                SDL_OpenGamepad().
 * \returns true if the gamepad has been opened and is currently connected, or
 *          false if not.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_GamepadConnected(SDL_Gamepad *gamepad);

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
 * Set the state of gamepad event processing.
 *
 * If gamepad events are disabled, you must call SDL_UpdateGamepads() yourself
 * and check the state of the gamepad when you want gamepad information.
 *
 * \param enabled whether to process gamepad events or not.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadEventsEnabled
 * \sa SDL_UpdateGamepads
 */
OVERRIDE void SDL_SetGamepadEventsEnabled(bool enabled);

/**
 * Query the state of gamepad event processing.
 *
 * If gamepad events are disabled, you must call SDL_UpdateGamepads() yourself
 * and check the state of the gamepad when you want gamepad information.
 *
 * \returns true if gamepad events are being processed, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetGamepadEventsEnabled
 */
OVERRIDE bool SDL_GamepadEventsEnabled(void);

/**
 *  Update the current state of the open game controllers.
 *
 *  This is called automatically by the event loop if any game controller
 *  events are enabled.
 */
OVERRIDE void SDL_GameControllerUpdate(void);

/**
 * Manually pump gamepad updates if not using the loop.
 *
 * This function is called automatically by the event loop if events are
 * enabled. Under such circumstances, it will not be necessary to call this
 * function.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE void SDL_UpdateGamepads(void);

/**
 * Get the SDL joystick layer binding for a controller axis mapping.
 *
 * \param gamecontroller a game controller
 * \param axis an axis enum value (one of the SDL_GameControllerAxis values)
 * \returns a SDL_GameControllerButtonBind describing the bind. On failure
 *          (like the given Controller axis doesn't exist on the device), its
 *          `.bindType` will be `SDL_CONTROLLER_BINDTYPE_NONE`.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GameControllerGetBindForButton
 */
OVERRIDE sdl2::SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(SDL_GameController *gamecontroller,
                                 sdl2::SDL_GameControllerAxis axis);

/**
 * Query whether a game controller has a given axis.
 *
 * This merely reports whether the controller's mapping defined this axis, as
 * that is all the information SDL has about the physical device.
 *
 * \param gamecontroller a game controller
 * \param axis an axis enum value (an SDL_GameControllerAxis value)
 * \returns SDL_TRUE if the controller has this axis, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerHasAxis(SDL_GameController *gamecontroller, sdl2::SDL_GameControllerAxis axis);

/**
 * Query whether a gamepad has a given axis.
 *
 * This merely reports whether the gamepad's mapping defined this axis, as
 * that is all the information SDL has about the physical device.
 *
 * \param gamepad a gamepad.
 * \param axis an axis enum value (an SDL_GamepadAxis value).
 * \returns true if the gamepad has this axis, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadHasButton
 * \sa SDL_GetGamepadAxis
 */
OVERRIDE bool SDL_GamepadHasAxis(SDL_Gamepad *gamepad, sdl3::SDL_GamepadAxis axis);

/**
 *  Get the current state of an axis control on a game controller.
 *
 *  The state is a value ranging from -32768 to 32767.
 *
 *  The axis indices start at index 0.
 */
OVERRIDE Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                                sdl2::SDL_GameControllerAxis axis);

/**
 * Get the current state of an axis control on a gamepad.
 *
 * The axis indices start at index 0.
 *
 * For thumbsticks, the state is a value ranging from -32768 (up/left) to
 * 32767 (down/right).
 *
 * Triggers range from 0 when released to 32767 when fully pressed, and never
 * return a negative value. Note that this differs from the value reported by
 * the lower-level SDL_GetJoystickAxis(), which normally uses the full range.
 *
 * \param gamepad a gamepad.
 * \param axis an axis index (one of the SDL_GamepadAxis values).
 * \returns axis state (including 0) on success or 0 (also) on failure; call
 *          SDL_GetError() for more information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadHasAxis
 * \sa SDL_GetGamepadButton
 */
OVERRIDE Sint16 SDL_GetGamepadAxis(SDL_Gamepad *gamepad, sdl3::SDL_GamepadAxis axis);
                                            
/**
 * Get the SDL joystick layer binding for a controller button mapping.
 *
 * \param gamecontroller a game controller
 * \param button an button enum value (an SDL_GameControllerButton value)
 * \returns a SDL_GameControllerButtonBind describing the bind. On failure
 *          (like the given Controller button doesn't exist on the device),
 *          its `.bindType` will be `SDL_CONTROLLER_BINDTYPE_NONE`.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GameControllerGetBindForAxis
 */
OVERRIDE sdl2::SDL_GameControllerButtonBind 
SDL_GameControllerGetBindForButton(SDL_GameController *gamecontroller,
                                   sdl2::SDL_GameControllerButton button);

/**
 * Query whether a game controller has a given button.
 *
 * This merely reports whether the controller's mapping defined this button,
 * as that is all the information SDL has about the physical device.
 *
 * \param gamecontroller a game controller
 * \param button a button enum value (an SDL_GameControllerButton value)
 * \returns SDL_TRUE if the controller has this button, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerHasButton(SDL_GameController *gamecontroller,
                                                             sdl2::SDL_GameControllerButton button);

/**
 * Query whether a gamepad has a given button.
 *
 * This merely reports whether the gamepad's mapping defined this button, as
 * that is all the information SDL has about the physical device.
 *
 * \param gamepad a gamepad.
 * \param button a button enum value (an SDL_GamepadButton value).
 * \returns true if the gamepad has this button, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadHasAxis
 */
OVERRIDE bool SDL_GamepadHasButton(SDL_Gamepad *gamepad, sdl3::SDL_GamepadButton button);
                                                      
/**
 *  Get the current state of a button on a game controller.
 *
 *  The button indices start at index 0.
 */
OVERRIDE Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 sdl2::SDL_GameControllerButton button);

/**
 * Get the current state of a button on a gamepad.
 *
 * \param gamepad a gamepad.
 * \param button a button index (one of the SDL_GamepadButton values).
 * \returns true if the button is pressed, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadHasButton
 * \sa SDL_GetGamepadAxis
 */
OVERRIDE bool SDL_GetGamepadButton(SDL_Gamepad *gamepad, sdl3::SDL_GamepadButton button);
                               
/**
 * Return whether a game controller has a particular sensor.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \returns SDL_TRUE if the sensor exists, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerHasSensor(SDL_GameController *gamecontroller, sdl2::SDL_SensorType type);

/**
 * Return whether a gamepad has a particular sensor.
 *
 * \param gamepad the gamepad to query.
 * \param type the type of sensor to query.
 * \returns true if the sensor exists, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGamepadSensorData
 * \sa SDL_GetGamepadSensorDataRate
 * \sa SDL_SetGamepadSensorEnabled
 */
OVERRIDE bool SDL_GamepadHasSensor(SDL_Gamepad *gamepad, sdl3::SDL_SensorType type);

/**
 * Set whether data reporting for a game controller sensor is enabled.
 *
 * \param gamecontroller The controller to update
 * \param type The type of sensor to enable/disable
 * \param enabled Whether data reporting should be enabled
 * \returns 0 or -1 if an error occurred.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE int SDL_GameControllerSetSensorEnabled(SDL_GameController *gamecontroller, sdl2::SDL_SensorType type, SDL_bool enabled);

/**
 * Set whether data reporting for a gamepad sensor is enabled.
 *
 * \param gamepad the gamepad to update.
 * \param type the type of sensor to enable/disable.
 * \param enabled whether data reporting should be enabled.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GamepadHasSensor
 * \sa SDL_GamepadSensorEnabled
 */
OVERRIDE bool SDL_SetGamepadSensorEnabled(SDL_Gamepad *gamepad, sdl3::SDL_SensorType type, bool enabled);

/**
 * Query whether sensor data reporting is enabled for a game controller.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \returns SDL_TRUE if the sensor is enabled, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerIsSensorEnabled(SDL_GameController *gamecontroller, sdl2::SDL_SensorType type);

/**
 * Query whether sensor data reporting is enabled for a gamepad.
 *
 * \param gamepad the gamepad to query.
 * \param type the type of sensor to query.
 * \returns true if the sensor is enabled, false otherwise.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetGamepadSensorEnabled
 */
OVERRIDE bool SDL_GamepadSensorEnabled(SDL_Gamepad *gamepad, sdl3::SDL_SensorType type);

/**
 * Get the data rate (number of events per second) of a game controller
 * sensor.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \return the data rate, or 0.0f if the data rate is not available.
 *
 * \since This function is available since SDL 2.0.16.
 */
OVERRIDE float SDL_GameControllerGetSensorDataRate(SDL_GameController *gamecontroller, sdl2::SDL_SensorType type);

/**
 * Get the data rate (number of events per second) of a gamepad sensor.
 *
 * \param gamepad the gamepad to query.
 * \param type the type of sensor to query.
 * \returns the data rate, or 0.0f if the data rate is not available.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE float SDL_GetGamepadSensorDataRate(SDL_Gamepad *gamepad, sdl3::SDL_SensorType type);

/**
 * Get the current state of a game controller sensor.
 *
 * The number of values and interpretation of the data is sensor dependent.
 * See SDL_sensor.h for the details for each type of sensor.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \param data A pointer filled with the current sensor state
 * \param num_values The number of values to write to data
 * \return 0 or -1 if an error occurred.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE int SDL_GameControllerGetSensorData(SDL_GameController *gamecontroller, sdl2::SDL_SensorType type, float *data, int num_values);

/**
 * Get the current state of a gamepad sensor.
 *
 * The number of values and interpretation of the data is sensor dependent.
 * See SDL_sensor.h for the details for each type of sensor.
 *
 * \param gamepad the gamepad to query.
 * \param type the type of sensor to query.
 * \param data a pointer filled with the current sensor state.
 * \param num_values the number of values to write to data.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_GetGamepadSensorData(SDL_Gamepad *gamepad, sdl3::SDL_SensorType type, float *data, int num_values);

/**
 *  Trigger a rumble effect
 *  Each call to this function cancels any previous rumble effect, and calling it with 0 intensity stops any rumbling.
 *
 *  \param gamecontroller The controller to vibrate
 *  \param low_frequency_rumble The intensity of the low frequency (left) rumble motor, from 0 to 0xFFFF
 *  \param high_frequency_rumble The intensity of the high frequency (right) rumble motor, from 0 to 0xFFFF
 *  \param duration_ms The duration of the rumble effect, in milliseconds
 *
 *  \return 0, or -1 if rumble isn't supported on this joystick
 */
OVERRIDE int SDL_GameControllerRumble(SDL_GameController *gamecontroller, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

/**
 * Start a rumble effect on a gamepad.
 *
 * Each call to this function cancels any previous rumble effect, and calling
 * it with 0 intensity stops any rumbling.
 *
 * This function requires you to process SDL events or call
 * SDL_UpdateJoysticks() to update rumble state.
 *
 * \param gamepad the gamepad to vibrate.
 * \param low_frequency_rumble the intensity of the low frequency (left)
 *                             rumble motor, from 0 to 0xFFFF.
 * \param high_frequency_rumble the intensity of the high frequency (right)
 *                              rumble motor, from 0 to 0xFFFF.
 * \param duration_ms the duration of the rumble effect, in milliseconds.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_RumbleGamepad(SDL_Gamepad *gamepad, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

/**
 * Start a rumble effect in the game controller's triggers.
 *
 * Each call to this function cancels any previous trigger rumble effect, and
 * calling it with 0 intensity stops any rumbling.
 *
 * Note that this is rumbling of the _triggers_ and not the game controller as
 * a whole. This is currently only supported on Xbox One controllers. If you
 * want the (more common) whole-controller rumble, use
 * SDL_GameControllerRumble() instead.
 *
 * \param gamecontroller The controller to vibrate
 * \param left_rumble The intensity of the left trigger rumble motor, from 0
 *                    to 0xFFFF
 * \param right_rumble The intensity of the right trigger rumble motor, from 0
 *                     to 0xFFFF
 * \param duration_ms The duration of the rumble effect, in milliseconds
 * \returns 0, or -1 if trigger rumble isn't supported on this controller
 *
 * \since This function is available since SDL 2.0.14.
 *
 * \sa SDL_GameControllerHasRumbleTriggers
 */
OVERRIDE int SDL_GameControllerRumbleTriggers(SDL_GameController *gamecontroller, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);

/**
 * Start a rumble effect in the gamepad's triggers.
 *
 * Each call to this function cancels any previous trigger rumble effect, and
 * calling it with 0 intensity stops any rumbling.
 *
 * Note that this is rumbling of the _triggers_ and not the gamepad as a
 * whole. This is currently only supported on Xbox One gamepads. If you want
 * the (more common) whole-gamepad rumble, use SDL_RumbleGamepad() instead.
 *
 * This function requires you to process SDL events or call
 * SDL_UpdateJoysticks() to update rumble state.
 *
 * \param gamepad the gamepad to vibrate.
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
 * \sa SDL_RumbleGamepad
 */
OVERRIDE bool SDL_RumbleGamepadTriggers(SDL_Gamepad *gamepad, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms);

/**
 * Query whether a game controller has an LED.
 *
 * \param gamecontroller The controller to query
 * \returns SDL_TRUE, or SDL_FALSE if this controller does not have a
 *          modifiable LED
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerHasLED(SDL_GameController *gamecontroller);

/**
 * Query whether a game controller has rumble support.
 *
 * \param gamecontroller The controller to query
 * \returns SDL_TRUE, or SDL_FALSE if this controller does not have rumble
 *          support
 *
 * \since This function is available since SDL 2.0.18.
 *
 * \sa SDL_GameControllerRumble
 */
OVERRIDE SDL_bool SDL_GameControllerHasRumble(SDL_GameController *gamecontroller);

/**
 * Query whether a game controller has rumble support on triggers.
 *
 * \param gamecontroller The controller to query
 * \returns SDL_TRUE, or SDL_FALSE if this controller does not have trigger
 *          rumble support
 *
 * \since This function is available since SDL 2.0.18.
 *
 * \sa SDL_GameControllerRumbleTriggers
 */
OVERRIDE SDL_bool SDL_GameControllerHasRumbleTriggers(SDL_GameController *gamecontroller);

/**
 * Update a game controller's LED color.
 *
 * \param gamecontroller The controller to update
 * \param red The intensity of the red LED
 * \param green The intensity of the green LED
 * \param blue The intensity of the blue LED
 * \returns 0, or -1 if this controller does not have a modifiable LED
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE int SDL_GameControllerSetLED(SDL_GameController *gamecontroller, Uint8 red, Uint8 green, Uint8 blue);

/**
 * Update a gamepad's LED color.
 *
 * An example of a joystick LED is the light on the back of a PlayStation 4's
 * DualShock 4 controller.
 *
 * For gamepads with a single color LED, the maximum of the RGB values will be
 * used as the LED brightness.
 *
 * \param gamepad the gamepad to update.
 * \param red the intensity of the red LED.
 * \param green the intensity of the green LED.
 * \param blue the intensity of the blue LED.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_SetGamepadLED(SDL_Gamepad *gamepad, Uint8 red, Uint8 green, Uint8 blue);

/**
 * Send a controller specific effect packet
 *
 * \param gamecontroller The controller to affect
 * \param data The data to send to the controller
 * \param size The size of the data to send to the controller
 * \returns 0, or -1 if this controller or driver doesn't support effect
 *          packets
 *
 * \since This function is available since SDL 2.0.16.
 */
OVERRIDE int SDL_GameControllerSendEffect(SDL_GameController *gamecontroller, const void *data, int size);

/**
 * Send a gamepad specific effect packet.
 *
 * \param gamepad the gamepad to affect.
 * \param data the data to send to the gamepad.
 * \param size the size of the data to send to the gamepad.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL 3.2.0.
 */
OVERRIDE bool SDL_SendGamepadEffect(SDL_Gamepad *gamepad, const void *data, int size);

/**
 *  Close a controller previously opened with SDL_GameControllerOpen().
 */
OVERRIDE void SDL_GameControllerClose(SDL_GameController *gamecontroller);

/**
 * Close a gamepad previously opened with SDL_OpenGamepad().
 *
 * \param gamepad a gamepad identifier previously returned by
 *                SDL_OpenGamepad().
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_OpenGamepad
 */
OVERRIDE void SDL_CloseGamepad(SDL_Gamepad *gamepad);

/* We don't need to hook these functions */
// SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForAxis(SDL2::SDL_GameControllerAxis axis);
// SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForButton(SDL2::SDL_GameControllerButton button);

}

#endif
