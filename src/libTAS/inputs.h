#ifndef INPUTS_H_INCL
#define INPUTS_H_INCL

#include "../external/SDL.h"
#include "global.h"

extern struct AllInputs ai;
extern struct AllInputs old_ai;

/* 
 * Declaring SDL_GameController, SDL_Joystick and SDL_Haptic structs
 * to be simply an int containing the controller id.
 * These structs are normally internal structs that are not
 * revealed by the API, so members of the struct won't be used
 * by the games.
 * The downside is that we have to implement all functions that
 * use these structs, otherwise we will probably get a crash.
 */
typedef int SDL_GameController;
typedef int SDL_Joystick;
typedef int SDL_Haptic;

/* A structure that encodes the stable unique id for a joystick device */
typedef struct {
    Uint8 data[16];
} SDL_JoystickGUID;

/* Keyboard functions */
OVERRIDE Uint8* SDL_GetKeyboardState(int* numkeys);
OVERRIDE Uint8* SDL_GetKeyState( int* numkeys);
int generateKeyUpEvent(void *events, void* gameWindow, int num, int update);
int generateKeyDownEvent(void *events, void* gameWindow, int num, int update);
int generateControllerEvent(SDL_Event* events, int num, int update);

/* Game controller functions */

/**
 *  Count the number of joysticks attached to the system right now
 */
OVERRIDE int SDL_NumJoysticks(void);

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
 *  turn this string into a axis mapping
 */
OVERRIDE SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString);

/**
 *  turn this axis enum into a string mapping
 */
OVERRIDE const char* SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis);

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
 *  turn this string into a button mapping
 */
OVERRIDE SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString);

/**
 *  turn this button enum into a string mapping
 */
OVERRIDE const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton button);

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


/*** Joystick devices ***/

/**
 *  Return the GUID for this opened joystick
 */
OVERRIDE SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick);



/*** Haptic devices ***/

/**
 *  \brief Count the number of haptic devices attached to the system.
 *
 *  \return Number of haptic devices detected on the system.
 */
OVERRIDE int SDL_NumHaptics(void);

/**
 *  \brief Opens a Haptic device for usage.
 *
 *  The index passed as an argument refers to the N'th Haptic device on this
 *  system.
 *
 *  When opening a haptic device, its gain will be set to maximum and
 *  autocenter will be disabled.  To modify these values use
 *  SDL_HapticSetGain() and SDL_HapticSetAutocenter().
 *
 *  \param device_index Index of the device to open.
 *  \return Device identifier or NULL on error.
 *
 *  \sa SDL_HapticIndex
 *  \sa SDL_HapticOpenFromMouse
 *  \sa SDL_HapticOpenFromJoystick
 *  \sa SDL_HapticClose
 *  \sa SDL_HapticSetGain
 *  \sa SDL_HapticSetAutocenter
 *  \sa SDL_HapticPause
 *  \sa SDL_HapticStopAll
 */
OVERRIDE SDL_Haptic * SDL_HapticOpen(int device_index);

/**
 *  \brief Checks to see if a joystick has haptic features.
 *
 *  \param joystick Joystick to test for haptic capabilities.
 *  \return 1 if the joystick is haptic, 0 if it isn't
 *          or -1 if an error ocurred.
 *
 *  \sa SDL_HapticOpenFromJoystick
 */
OVERRIDE int SDL_JoystickIsHaptic(SDL_Joystick * joystick);

/**
 *  \brief Opens a Haptic device for usage from a Joystick device.
 *
 *  You must still close the haptic device separately.  It will not be closed
 *  with the joystick.
 *
 *  When opening from a joystick you should first close the haptic device before
 *  closing the joystick device.  If not, on some implementations the haptic
 *  device will also get unallocated and you'll be unable to use force feedback
 *  on that device.
 *
 *  \param joystick Joystick to create a haptic device from.
 *  \return A valid haptic device identifier on success or NULL on error.
 *
 *  \sa SDL_HapticOpen
 *  \sa SDL_HapticClose
 */
OVERRIDE SDL_Haptic *SDL_HapticOpenFromJoystick(SDL_Joystick *joystick);

/**
 *  \brief Closes a Haptic device previously opened with SDL_HapticOpen().
 *
 *  \param haptic Haptic device to close.
 */
OVERRIDE void SDL_HapticClose(SDL_Haptic * haptic);

#endif // INPUTS_H_INCL
