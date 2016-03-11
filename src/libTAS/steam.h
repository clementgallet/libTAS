#include "global.h"

OVERRIDE void SteamAPI_Shutdown();

// checks if a local Steam client is running 
OVERRIDE bool SteamAPI_IsSteamRunning();

// Detects if your executable was launched through the Steam client, and restarts your game through 
// the client if necessary. The Steam client will be started if it is not running.
//
// Returns: true if your executable was NOT launched through the Steam client. This function will
//          then start your application through the client. Your current process should exit.
//
//          false if your executable was started through the Steam client or a steam_appid.txt file
//          is present in your game's directory (for development). Your current process should continue.
//
// NOTE: This function should be used only if you are using CEG or not using Steam's DRM. Once applied
//       to your executable, Steam's DRM will handle restarting through Steam if necessary.
OVERRIDE bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID );

OVERRIDE bool SteamAPI_Init();

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steam callback helper functions
//
//	The following classes/macros are used to be able to easily multiplex callbacks 
//	from the Steam API into various objects in the app in a thread-safe manner
//
//	These functors are triggered via the SteamAPI_RunCallbacks() function, mapping the callback
//  to as many functions/objects as are registered to it
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

OVERRIDE void SteamAPI_RunCallbacks();

// functions used by the utility CCallback objects to receive callbacks
OVERRIDE void SteamAPI_RegisterCallback( void *pCallback, int iCallback );
OVERRIDE void SteamAPI_UnregisterCallback( void *pCallback );

