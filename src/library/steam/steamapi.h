/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_STEAMAPI_H_INCL
#define LIBTAS_STEAMAPI_H_INCL

#include "../global.h"
#include "isteamclient.h"
#include "isteamcontroller.h"
#include "isteamuser.h"
#include "isteamuserstats.h"
#include "isteamutils.h"
#include "isteamremotestorage.h"
#include "isteamapps.h"
#include "isteamfriends.h"
#include "isteamscreenshots.h"
#include "isteamugc.h"

namespace libtas {

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

OVERRIDE bool SteamAPI_InitSafe();

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


OVERRIDE ISteamClient *SteamClient();
OVERRIDE ISteamController *SteamController();
OVERRIDE ISteamUserStats *SteamUserStats();
OVERRIDE ISteamUser *SteamUser();
OVERRIDE ISteamUtils *SteamUtils();
OVERRIDE ISteamRemoteStorage *SteamRemoteStorage();
OVERRIDE ISteamApps *SteamApps();
OVERRIDE ISteamFriends *SteamFriends();
OVERRIDE ISteamScreenshots *SteamScreenshots();
OVERRIDE ISteamUGC *SteamUGC();

}

#endif
