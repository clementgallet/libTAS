//====== Copyright 1996-2013, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to valve controller
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCONTROLLER_H_INCL
#define LIBTAS_ISTEAMCONTROLLER_H_INCL

#include <stdint.h>

namespace libtas {

// When sending an option to a specific controller handle, you can send to all controllers via this command
#define STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS UINT64_MAX

enum ESteamControllerPad
{
	k_ESteamControllerPad_Left,
	k_ESteamControllerPad_Right
};

enum EControllerSource
{
	k_EControllerSource_None,
	k_EControllerSource_LeftTrackpad,
	k_EControllerSource_RightTrackpad,
	k_EControllerSource_Joystick,
	k_EControllerSource_ABXY,
	k_EControllerSource_Switch,
	k_EControllerSource_LeftTrigger,
	k_EControllerSource_RightTrigger,
	k_EControllerSource_Gyro
};

enum EControllerSourceMode
{
	k_EControllerSourceMode_None,
	k_EControllerSourceMode_Dpad,
	k_EControllerSourceMode_Buttons,
	k_EControllerSourceMode_FourButtons,
	k_EControllerSourceMode_AbsoluteMouse,
	k_EControllerSourceMode_RelativeMouse,
	k_EControllerSourceMode_JoystickMove,
	k_EControllerSourceMode_JoystickCamera,
	k_EControllerSourceMode_ScrollWheel,
	k_EControllerSourceMode_Trigger,
	k_EControllerSourceMode_TouchMenu
};

enum EControllerActionOrigin
{
	k_EControllerActionOrigin_None = 0,
	k_EControllerActionOrigin_A = 1, // (Valve Steam Controller) digital face button A
	k_EControllerActionOrigin_B = 2, // (Valve Steam Controller) digital face button B
	k_EControllerActionOrigin_X = 3, // (Valve Steam Controller) digital face button X
	k_EControllerActionOrigin_Y = 4, // (Valve Steam Controller) digital face button Y
	k_EControllerActionOrigin_LeftBumper = 5, // (Valve Steam Controller) digital left shoulder button (aka "left bumper")
	k_EControllerActionOrigin_RightBumper = 6, // (Valve Steam Controller) digital right shoulder button (aka "right bumper")
	k_EControllerActionOrigin_LeftGrip = 7, // (Valve Steam Controller) digital left grip paddle
	k_EControllerActionOrigin_RightGrip = 8, // (Valve Steam Controller) digital right grip paddle
	k_EControllerActionOrigin_Start = 9, // (Valve Steam Controller) digital start button
	k_EControllerActionOrigin_Back = 10, // (Valve Steam Controller) digital back button
	k_EControllerActionOrigin_LeftPad_Touch = 11, // (Valve Steam Controller) left haptic touchpad, in simple contact with a finger
	k_EControllerActionOrigin_LeftPad_Swipe = 12, // (Valve Steam Controller) left haptic touchpad, touch input on any axis
	k_EControllerActionOrigin_LeftPad_Click = 13, // (Valve Steam Controller) left haptic touchpad, digital click (for the whole thing)
	k_EControllerActionOrigin_LeftPad_DPadNorth = 14, // (Valve Steam Controller) left haptic touchpad, digital click (upper quadrant)
	k_EControllerActionOrigin_LeftPad_DPadSouth = 15, // (Valve Steam Controller) left haptic touchpad, digital click (lower quadrant)
	k_EControllerActionOrigin_LeftPad_DPadWest = 16, // (Valve Steam Controller) left haptic touchpad, digital click (left quadrant)
	k_EControllerActionOrigin_LeftPad_DPadEast = 17, // (Valve Steam Controller) left haptic touchpad, digital click (right quadrant)
	k_EControllerActionOrigin_RightPad_Touch = 18, // (Valve Steam Controller) right haptic touchpad, in simple contact with a finger
	k_EControllerActionOrigin_RightPad_Swipe = 19, // (Valve Steam Controller) right haptic touchpad, touch input on any axis
	k_EControllerActionOrigin_RightPad_Click = 20, // (Valve Steam Controller) right haptic touchpad, digital click (for the whole thing)
	k_EControllerActionOrigin_RightPad_DPadNorth = 21, // (Valve Steam Controller) right haptic touchpad, digital click (upper quadrant)
	k_EControllerActionOrigin_RightPad_DPadSouth = 22, // (Valve Steam Controller) right haptic touchpad, digital click (lower quadrant)
	k_EControllerActionOrigin_RightPad_DPadWest = 23, // (Valve Steam Controller) right haptic touchpad, digital click (left quadrant)
	k_EControllerActionOrigin_RightPad_DPadEast = 24, // (Valve Steam Controller) right haptic touchpad, digital click (right quadrant)
	k_EControllerActionOrigin_LeftTrigger_Pull = 25, // (Valve Steam Controller) left analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_LeftTrigger_Click = 26, // (Valve Steam Controller) left analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_RightTrigger_Pull = 27, // (Valve Steam Controller) right analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_RightTrigger_Click = 28, // (Valve Steam Controller) right analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_LeftStick_Move = 29, // (Valve Steam Controller) left joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_LeftStick_Click = 30, // (Valve Steam Controller) left joystick, clicked in (digital value)
	k_EControllerActionOrigin_LeftStick_DPadNorth = 31, // (Valve Steam Controller) left joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_LeftStick_DPadSouth = 32, // (Valve Steam Controller) left joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_LeftStick_DPadWest = 33, // (Valve Steam Controller) left joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_LeftStick_DPadEast = 34, // (Valve Steam Controller) left joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_Gyro_Move = 35, // (Valve Steam Controller) gyroscope, analog movement in any axis
	k_EControllerActionOrigin_Gyro_Pitch = 36, // (Valve Steam Controller) gyroscope, analog movement on the Pitch axis (point head up to ceiling, point head down to floor)
	k_EControllerActionOrigin_Gyro_Yaw = 37, // (Valve Steam Controller) gyroscope, analog movement on the Yaw axis (turn head left to face one wall, turn head right to face other)
	k_EControllerActionOrigin_Gyro_Roll = 38, // (Valve Steam Controller) gyroscope, analog movement on the Roll axis (tilt head left towards shoulder, tilt head right towards other)
	k_EControllerActionOrigin_PS4_X = 39, // (Sony Dualshock 4) digital face button X
	k_EControllerActionOrigin_PS4_Circle = 40, // (Sony Dualshock 4) digital face button Circle
	k_EControllerActionOrigin_PS4_Triangle = 41, // (Sony Dualshock 4) digital face button Triangle
	k_EControllerActionOrigin_PS4_Square = 42, // (Sony Dualshock 4) digital face button Square
	k_EControllerActionOrigin_PS4_LeftBumper = 43, // (Sony Dualshock 4) digital left shoulder button (aka "left bumper")
	k_EControllerActionOrigin_PS4_RightBumper = 44, // (Sony Dualshock 4) digital right shoulder button (aka "right bumper")
	k_EControllerActionOrigin_PS4_Options = 45, // (Sony Dualshock 4) digital options button (aka "Start")
	k_EControllerActionOrigin_PS4_Share = 46, // (Sony Dualshock 4) digital share button (aka "Back")
	k_EControllerActionOrigin_PS4_LeftPad_Touch = 47, // (Sony Dualshock 4) left half of the touchpad, in simple contact with a finger
	k_EControllerActionOrigin_PS4_LeftPad_Swipe = 48, // (Sony Dualshock 4) left half of the touchpad, touch input on any axis
	k_EControllerActionOrigin_PS4_LeftPad_Click = 49, // (Sony Dualshock 4) left half of the touchpad, digital click (for the whole thing)
	k_EControllerActionOrigin_PS4_LeftPad_DPadNorth = 50, // (Sony Dualshock 4) left half of the touchpad, digital click (upper quadrant)
	k_EControllerActionOrigin_PS4_LeftPad_DPadSouth = 51, // (Sony Dualshock 4) left half of the touchpad, digital click (lower quadrant)
	k_EControllerActionOrigin_PS4_LeftPad_DPadWest = 52, // (Sony Dualshock 4) left half of the touchpad, digital click (left quadrant)
	k_EControllerActionOrigin_PS4_LeftPad_DPadEast = 53, // (Sony Dualshock 4) left half of the touchpad, digital click (right quadrant)
	k_EControllerActionOrigin_PS4_RightPad_Touch = 54, // (Sony Dualshock 4) right half of the touchpad, in simple contact with a finger
	k_EControllerActionOrigin_PS4_RightPad_Swipe = 55, // (Sony Dualshock 4) right half of the touchpad, touch input on any axis
	k_EControllerActionOrigin_PS4_RightPad_Click = 56, // (Sony Dualshock 4) right half of the touchpad, digital click (for the whole thing)
	k_EControllerActionOrigin_PS4_RightPad_DPadNorth = 57, // (Sony Dualshock 4) right half of the touchpad, digital click (upper quadrant)
	k_EControllerActionOrigin_PS4_RightPad_DPadSouth = 58, // (Sony Dualshock 4) right half of the touchpad, digital click (lower quadrant)
	k_EControllerActionOrigin_PS4_RightPad_DPadWest = 59, // (Sony Dualshock 4) right half of the touchpad, digital click (left quadrant)
	k_EControllerActionOrigin_PS4_RightPad_DPadEast = 60, // (Sony Dualshock 4) right half of the touchpad, digital click (right quadrant)
	k_EControllerActionOrigin_PS4_CenterPad_Touch = 61, // (Sony Dualshock 4) unified touchpad, in simple contact with a finger
	k_EControllerActionOrigin_PS4_CenterPad_Swipe = 62, // (Sony Dualshock 4) unified touchpad, touch input on any axis
	k_EControllerActionOrigin_PS4_CenterPad_Click = 63, // (Sony Dualshock 4) unified touchpad, digital click (for the whole thing)
	k_EControllerActionOrigin_PS4_CenterPad_DPadNorth = 64, // (Sony Dualshock 4) unified touchpad, digital click (upper quadrant)
	k_EControllerActionOrigin_PS4_CenterPad_DPadSouth = 65, // (Sony Dualshock 4) unified touchpad, digital click (lower quadrant)
	k_EControllerActionOrigin_PS4_CenterPad_DPadWest = 66, // (Sony Dualshock 4) unified touchpad, digital click (left quadrant)
	k_EControllerActionOrigin_PS4_CenterPad_DPadEast = 67, // (Sony Dualshock 4) unified touchpad, digital click (right quadrant)
	k_EControllerActionOrigin_PS4_LeftTrigger_Pull = 68, // (Sony Dualshock 4) left analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_PS4_LeftTrigger_Click = 69, // (Sony Dualshock 4) left analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_PS4_RightTrigger_Pull = 70, // (Sony Dualshock 4) right analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_PS4_RightTrigger_Click = 71, // (Sony Dualshock 4) right analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_PS4_LeftStick_Move = 72, // (Sony Dualshock 4) left joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_PS4_LeftStick_Click = 73, // (Sony Dualshock 4) left joystick, clicked in (digital value)
	k_EControllerActionOrigin_PS4_LeftStick_DPadNorth = 74, // (Sony Dualshock 4) left joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_PS4_LeftStick_DPadSouth = 75, // (Sony Dualshock 4) left joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_PS4_LeftStick_DPadWest = 76, // (Sony Dualshock 4) left joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_PS4_LeftStick_DPadEast = 77, // (Sony Dualshock 4) left joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_PS4_RightStick_Move = 78, // (Sony Dualshock 4) right joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_PS4_RightStick_Click = 79, // (Sony Dualshock 4) right joystick, clicked in (digital value)
	k_EControllerActionOrigin_PS4_RightStick_DPadNorth = 80, // (Sony Dualshock 4) right joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_PS4_RightStick_DPadSouth = 81, // (Sony Dualshock 4) right joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_PS4_RightStick_DPadWest = 82, // (Sony Dualshock 4) right joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_PS4_RightStick_DPadEast = 83, // (Sony Dualshock 4) right joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_PS4_DPad_North = 84, // (Sony Dualshock 4) digital pad, pressed (upper quadrant)
	k_EControllerActionOrigin_PS4_DPad_South = 85, // (Sony Dualshock 4) digital pad, pressed (lower quadrant)
	k_EControllerActionOrigin_PS4_DPad_West = 86, // (Sony Dualshock 4) digital pad, pressed (left quadrant)
	k_EControllerActionOrigin_PS4_DPad_East = 87, // (Sony Dualshock 4) digital pad, pressed (right quadrant)
	k_EControllerActionOrigin_PS4_Gyro_Move = 88, // (Sony Dualshock 4) gyroscope, analog movement in any axis
	k_EControllerActionOrigin_PS4_Gyro_Pitch = 89, // (Sony Dualshock 4) gyroscope, analog movement on the Pitch axis (point head up to ceiling, point head down to floor)
	k_EControllerActionOrigin_PS4_Gyro_Yaw = 90, // (Sony Dualshock 4) gyroscope, analog movement on the Yaw axis (turn head left to face one wall, turn head right to face other)
	k_EControllerActionOrigin_PS4_Gyro_Roll = 91, // (Sony Dualshock 4) gyroscope, analog movement on the Roll axis (tilt head left towards shoulder, tilt head right towards other shoulder)
	k_EControllerActionOrigin_XBoxOne_A = 92, // (XB1) digital face button A
	k_EControllerActionOrigin_XBoxOne_B = 93, // (XB1) digital face button B
	k_EControllerActionOrigin_XBoxOne_X = 94, // (XB1) digital face button X
	k_EControllerActionOrigin_XBoxOne_Y = 95, // (XB1) digital face button Y
	k_EControllerActionOrigin_XBoxOne_LeftBumper = 96, // (XB1) digital left shoulder button (aka "left bumper")
	k_EControllerActionOrigin_XBoxOne_RightBumper = 97, // (XB1) digital right shoulder button (aka "right bumper")
	k_EControllerActionOrigin_XBoxOne_Menu = 98, // (XB1) digital menu button (aka "start")
	k_EControllerActionOrigin_XBoxOne_View = 99, // (XB1) digital view button (aka "back")
	k_EControllerActionOrigin_XBoxOne_LeftTrigger_Pull = 100, // (XB1) left analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_XBoxOne_LeftTrigger_Click = 101, // (XB1) left analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_XBoxOne_RightTrigger_Pull = 102, // (XB1) right analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_XBoxOne_RightTrigger_Click = 103, // (XB1) right analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_XBoxOne_LeftStick_Move = 104, // (XB1) left joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_XBoxOne_LeftStick_Click = 105, // (XB1) left joystick, clicked in (digital value)
	k_EControllerActionOrigin_XBoxOne_LeftStick_DPadNorth = 106, // (XB1) left joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_XBoxOne_LeftStick_DPadSouth = 107, // (XB1) left joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_XBoxOne_LeftStick_DPadWest = 108, // (XB1) left joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_XBoxOne_LeftStick_DPadEast = 109, // (XB1) left joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_XBoxOne_RightStick_Move = 110, // (XB1) right joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_XBoxOne_RightStick_Click = 111, // (XB1) right joystick, clicked in (digital value)
	k_EControllerActionOrigin_XBoxOne_RightStick_DPadNorth = 112, // (XB1) right joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_XBoxOne_RightStick_DPadSouth = 113, // (XB1) right joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_XBoxOne_RightStick_DPadWest = 114, // (XB1) right joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_XBoxOne_RightStick_DPadEast = 115, // (XB1) right joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_XBoxOne_DPad_North = 116, // (XB1) digital pad, pressed (upper quadrant)
	k_EControllerActionOrigin_XBoxOne_DPad_South = 117, // (XB1) digital pad, pressed (lower quadrant)
	k_EControllerActionOrigin_XBoxOne_DPad_West = 118, // (XB1) digital pad, pressed (left quadrant)
	k_EControllerActionOrigin_XBoxOne_DPad_East = 119, // (XB1) digital pad, pressed (right quadrant)
	k_EControllerActionOrigin_XBox360_A = 120, // (X360) digital face button A
	k_EControllerActionOrigin_XBox360_B = 121, // (X360) digital face button B
	k_EControllerActionOrigin_XBox360_X = 122, // (X360) digital face button X
	k_EControllerActionOrigin_XBox360_Y = 123, // (X360) digital face button Y
	k_EControllerActionOrigin_XBox360_LeftBumper = 124, // (X360) digital left shoulder button (aka "left bumper")
	k_EControllerActionOrigin_XBox360_RightBumper = 125, // (X360) digital right shoulder button (aka "right bumper")
	k_EControllerActionOrigin_XBox360_Start = 126, // (X360) digital start button
	k_EControllerActionOrigin_XBox360_Back = 127, // (X360) digital back button
	k_EControllerActionOrigin_XBox360_LeftTrigger_Pull = 128, // (X360) left analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_XBox360_LeftTrigger_Click = 129, // (X360) left analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_XBox360_RightTrigger_Pull = 130, // (X360) right analog trigger, pulled by any amount (analog value)
	k_EControllerActionOrigin_XBox360_RightTrigger_Click = 131, // (X360) right analog trigger, pulled in all the way (digital value)
	k_EControllerActionOrigin_XBox360_LeftStick_Move = 132, // (X360) left joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_XBox360_LeftStick_Click = 133, // (X360) left joystick, clicked in (digital value)
	k_EControllerActionOrigin_XBox360_LeftStick_DPadNorth = 134, // (X360) left joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_XBox360_LeftStick_DPadSouth = 135, // (X360) left joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_XBox360_LeftStick_DPadWest = 136, // (X360) left joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_XBox360_LeftStick_DPadEast = 137, // (X360) left joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_XBox360_RightStick_Move = 138, // (X360) right joystick, movement on any axis (analog value)
	k_EControllerActionOrigin_XBox360_RightStick_Click = 139, // (X360) right joystick, clicked in (digital value)
	k_EControllerActionOrigin_XBox360_RightStick_DPadNorth = 140, // (X360) right joystick, digital movement (upper quadrant)
	k_EControllerActionOrigin_XBox360_RightStick_DPadSouth = 141, // (X360) right joystick, digital movement (lower quadrant)
	k_EControllerActionOrigin_XBox360_RightStick_DPadWest = 142, // (X360) right joystick, digital movement (left quadrant)
	k_EControllerActionOrigin_XBox360_RightStick_DPadEast = 143, // (X360) right joystick, digital movement (right quadrant)
	k_EControllerActionOrigin_XBox360_DPad_North = 144, // (X360) digital pad, pressed (upper quadrant)
	k_EControllerActionOrigin_XBox360_DPad_South = 145, // (X360) digital pad, pressed (lower quadrant)
	k_EControllerActionOrigin_XBox360_DPad_West = 146, // (X360) digital pad, pressed (left quadrant)
	k_EControllerActionOrigin_XBox360_DPad_East = 147, // (X360) digital pad, pressed (right quadrant)
	k_EControllerActionOrigin_Count = 196, // The number of values in this enum, useful for iterating.
};

// ControllerHandle_t is used to refer to a specific controller.
// This handle will consistently identify a controller, even if it is disconnected and re-connected
typedef uint64_t ControllerHandle_t;


// These handles are used to refer to a specific in-game action or action set
// All action handles should be queried during initialization for performance reasons
typedef uint64_t ControllerActionSetHandle_t;
typedef uint64_t ControllerDigitalActionHandle_t;
typedef uint64_t ControllerAnalogActionHandle_t;

#pragma pack( push, 1 )

struct ControllerAnalogActionData_t
{
	// Type of data coming from this action, this will match what got specified in the action set
	EControllerSourceMode eMode;

	// The current state of this action; will be delta updates for mouse actions
	float x, y;

	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;
};

struct ControllerDigitalActionData_t
{
	// The current state of this action; will be true if currently pressed
	bool bState;

	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;
};

#pragma pack( pop )


//-----------------------------------------------------------------------------
// Purpose: Native Steam controller support API
//-----------------------------------------------------------------------------
class ISteamController
{
public:

	// Init and Shutdown must be called when starting/ending use of this interface
	virtual bool Init();
	virtual bool Shutdown();

	// Pump callback/callresult events
	// Note: SteamAPI_RunCallbacks will do this for you, so you should never need to call this directly.
	virtual void RunFrame();

	// Enumerate currently connected controllers
	// handlesOut should point to a STEAM_CONTROLLER_MAX_COUNT sized array of ControllerHandle_t handles
	// Returns the number of handles written to handlesOut
	virtual int GetConnectedControllers( ControllerHandle_t *handlesOut );

	// Invokes the Steam overlay and brings up the binding screen
	// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
	virtual bool ShowBindingPanel( ControllerHandle_t controllerHandle );

	// ACTION SETS
	// Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerActionSetHandle_t GetActionSetHandle( const char *pszActionSetName );

	// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
	// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
	// your state loops, instead of trying to place it in all of your state transitions.
	virtual void ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle );
	virtual ControllerActionSetHandle_t GetCurrentActionSet( ControllerHandle_t controllerHandle );

	// ACTIONS
	// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerDigitalActionHandle_t GetDigitalActionHandle( const char *pszActionName );

	// Returns the current state of the supplied digital game action
	virtual ControllerDigitalActionData_t GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle );

	// Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
	virtual int GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut );

	// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName );

	// Returns the current state of these supplied analog game action
	virtual ControllerAnalogActionData_t GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle );

	// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
	virtual int GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut );



	virtual void StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction );

	// Trigger a haptic pulse on a controller
	virtual void TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );
};

}

#endif // ISTEAMCONTROLLER_H
