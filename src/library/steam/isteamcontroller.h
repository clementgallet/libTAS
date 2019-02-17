//====== Copyright 1996-2013, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to valve controller
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCONTROLLER_H_INCL
#define LIBTAS_ISTEAMCONTROLLER_H_INCL

#include "steamtypes.h"
#include "steamcontrollerpublic.h"

namespace libtas {

#define MAX_STEAM_CONTROLLERS 16

enum ESteamControllerPad
{
	k_ESteamControllerPad_Left,
	k_ESteamControllerPad_Right
};

//-----------------------------------------------------------------------------
// Purpose: Native Steam controller support API
//-----------------------------------------------------------------------------
class ISteamController
{
public:

	//
	// Native controller support API
	//

	// Must call init and shutdown when starting/ending use of the interface
	virtual bool Init( const char *pchAbsolutePathToControllerConfigVDF );
	virtual bool Shutdown();

	// Pump callback/callresult events, SteamAPI_RunCallbacks will do this for you,
	// normally never need to call directly.
	virtual void RunFrame();

	// Get the state of the specified controller, returns false if that controller is not connected
	virtual bool GetControllerState( uint32_t unControllerIndex, SteamControllerState_t *pState );

	// Trigger a haptic pulse on the controller
	virtual void TriggerHapticPulse( uint32_t unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );

	// Set the override mode which is used to choose to use different base/legacy bindings from your config file
	virtual void SetOverrideMode( const char *pchMode );
};

#define STEAMCONTROLLER_INTERFACE_VERSION "STEAMCONTROLLER_INTERFACE_VERSION"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*
struct ControllerCallback_t
{
	enum { k_iCallback = k_iSteamControllerCallbacks + 1 };
};
*/

}

#endif // ISTEAMCONTROLLER_H
