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

#ifndef LIBTAS_SDLGAMECONTROLLER_H_INCL
#define LIBTAS_SDLGAMECONTROLLER_H_INCL

#include "sdljoystick.h"
#include <SDL2/SDL.h>
#include "../global.h"

namespace libtas {

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
 *  Get a mapping string for a GUID
 *
 *  \return the mapping string.  Must be freed with SDL_free.  Returns NULL if no mapping is available
 */
OVERRIDE char *SDL_GameControllerMappingForGUID( SDL_JoystickGUID guid );

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

typedef enum
{
    SDL_CONTROLLER_TYPE_UNKNOWN = 0,
    SDL_CONTROLLER_TYPE_XBOX360,
    SDL_CONTROLLER_TYPE_XBOXONE,
    SDL_CONTROLLER_TYPE_PS3,
    SDL_CONTROLLER_TYPE_PS4,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO,
    SDL_CONTROLLER_TYPE_VIRTUAL,
    SDL_CONTROLLER_TYPE_PS5,
    SDL_CONTROLLER_TYPE_AMAZON_LUNA,
    SDL_CONTROLLER_TYPE_GOOGLE_STADIA
} SDL_GameControllerType;

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
OVERRIDE SDL_GameControllerType SDL_GameControllerTypeForIndex(int joystick_index);

/**
 *  Return the name for this currently opened controller
 */
OVERRIDE const char *SDL_GameControllerName(SDL_GameController *gamecontroller);

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
OVERRIDE SDL_GameControllerType SDL_GameControllerGetType(SDL_GameController *gamecontroller);

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

typedef enum
{
    SDL_SENSOR_INVALID = -1,    /**< Returned for an invalid sensor */
    SDL_SENSOR_UNKNOWN,         /**< Unknown sensor type */
    SDL_SENSOR_ACCEL,           /**< Accelerometer */
    SDL_SENSOR_GYRO             /**< Gyroscope */
} SDL_SensorType;

/**
 * Return whether a game controller has a particular sensor.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \returns SDL_TRUE if the sensor exists, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerHasSensor(SDL_GameController *gamecontroller, SDL_SensorType type);

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
OVERRIDE int SDL_GameControllerSetSensorEnabled(SDL_GameController *gamecontroller, SDL_SensorType type, SDL_bool enabled);

/**
 * Query whether sensor data reporting is enabled for a game controller.
 *
 * \param gamecontroller The controller to query
 * \param type The type of sensor to query
 * \returns SDL_TRUE if the sensor is enabled, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.14.
 */
OVERRIDE SDL_bool SDL_GameControllerIsSensorEnabled(SDL_GameController *gamecontroller, SDL_SensorType type);

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
OVERRIDE float SDL_GameControllerGetSensorDataRate(SDL_GameController *gamecontroller, SDL_SensorType type);

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
OVERRIDE int SDL_GameControllerGetSensorData(SDL_GameController *gamecontroller, SDL_SensorType type, float *data, int num_values);

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
 *  Close a controller previously opened with SDL_GameControllerOpen().
 */
OVERRIDE void SDL_GameControllerClose(SDL_GameController *gamecontroller);

/* We don't need to hook these functions */
// SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis);
// SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString);
// const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton button);

}

#endif
