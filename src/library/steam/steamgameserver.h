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

#ifndef LIBTAS_STEAMGAMESERVER_H_INCL
#define LIBTAS_STEAMGAMESERVER_H_INCL

#include "../global.h"
#include "isteamclient/isteamclient.h"
#include "isteamuser.h"
#include "isteamutils.h"
#include "isteamapps.h"
#include "isteamugc.h"
#include "isteamhttp.h"
#include "isteamgameserver.h"
#include "isteamnetworking.h"

namespace libtas {

// Initialize ISteamGameServer interface object, and set server properties which may not be changed.
//
// After calling this function, you should set any additional server parameters, and then
// call ISteamGameServer::LogOnAnonymous() or ISteamGameServer::LogOn()
//
// - usSteamPort is the local port used to communicate with the steam servers.
// - usGamePort is the port that clients will connect to for gameplay.
// - usQueryPort is the port that will manage server browser related duties and info
//		pings from clients.  If you pass MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE for usQueryPort, then it
//		will use "GameSocketShare" mode, which means that the game is responsible for sending and receiving
//		UDP packets for the master  server updater. See references to GameSocketShare in isteamgameserver.h.
// - The version string is usually in the form x.x.x.x, and is used by the master server to detect when the
//		server is out of date.  (Only servers with the latest version will be listed.)

OVERRIDE bool SteamGameServer_Init( uint32_t unIP, uint16_t usSteamPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString );

OVERRIDE void SteamGameServer_Shutdown();
OVERRIDE void SteamGameServer_RunCallbacks();

// Most Steam API functions allocate some amount of thread-local memory for
// parameter storage. Calling SteamGameServer_ReleaseCurrentThreadMemory()
// will free all API-related memory associated with the calling thread.
// This memory is released automatically by SteamGameServer_RunCallbacks(),
// so single-threaded servers do not need to explicitly call this function.
OVERRIDE void SteamGameServer_ReleaseCurrentThreadMemory();

OVERRIDE bool SteamGameServer_BSecure();
OVERRIDE uint64_t SteamGameServer_GetSteamID();

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
// Global accessors for game server C++ APIs. See individual isteam*.h files for details.
// You should not cache the results of these accessors or pass the result pointers across
// modules! Different modules may be compiled against different SDK header versions, and
// the interface pointers could therefore be different across modules. Every line of code
// which calls into a Steamworks API should retrieve the interface from a global accessor.
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

typedef void ISteamGameServerStats;
typedef void ISteamInventory;

OVERRIDE ISteamClient *SteamGameServerClient();
OVERRIDE ISteamGameServer *SteamGameServer();
OVERRIDE ISteamUtils *SteamGameServerUtils();
OVERRIDE ISteamNetworking *SteamGameServerNetworking();
OVERRIDE ISteamGameServerStats *SteamGameServerStats();
OVERRIDE ISteamHTTP *SteamGameServerHTTP();
OVERRIDE ISteamInventory *SteamGameServerInventory();
OVERRIDE ISteamUGC *SteamGameServerUGC();
OVERRIDE ISteamApps *SteamGameServerApps();

class CSteamGameServerAPIContext
{
public:
	// ISteamClient *SteamClient() const					{ return m_pSteamClient; }
	// ISteamGameServer *SteamGameServer() const			{ return m_pSteamGameServer; }
	// ISteamUtils *SteamGameServerUtils() const			{ return m_pSteamGameServerUtils; }
	// ISteamNetworking *SteamGameServerNetworking() const	{ return m_pSteamGameServerNetworking; }
	// ISteamGameServerStats *SteamGameServerStats() const	{ return m_pSteamGameServerStats; }
	// ISteamHTTP *SteamHTTP() const						{ return m_pSteamHTTP; }
	// ISteamInventory *SteamInventory() const				{ return m_pSteamInventory; }
	// ISteamUGC *SteamUGC() const							{ return m_pSteamUGC; }
	// ISteamApps *SteamApps() const						{ return m_pSteamApps; }

private:
	ISteamClient				*m_pSteamClient;
	ISteamGameServer			*m_pSteamGameServer;
	ISteamUtils					*m_pSteamGameServerUtils;
	ISteamNetworking			*m_pSteamGameServerNetworking;
	ISteamGameServerStats		*m_pSteamGameServerStats;
	ISteamHTTP					*m_pSteamHTTP;
	ISteamInventory				*m_pSteamInventory;
	ISteamUGC					*m_pSteamUGC;
	ISteamApps					*m_pSteamApps;
};


// Older SDKs exported this global pointer, but it is no longer supported.
// You should use SteamGameServerClient() or CSteamGameServerAPIContext to
// safely access the ISteamClient APIs from your game server application.
//S_API ISteamClient *g_pSteamClientGameServer;

// SteamGameServer_InitSafe has been replaced with SteamGameServer_Init and
// is no longer supported. Use SteamGameServer_Init instead.
//S_API void S_CALLTYPE SteamGameServer_InitSafe();

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steamclient.dll private wrapper functions
//
//	The following functions are part of abstracting API access to the steamclient.dll, but should only be used in very specific cases
//----------------------------------------------------------------------------------------------------------------------------------------------------------//
OVERRIDE HSteamPipe SteamGameServer_GetHSteamPipe();
OVERRIDE HSteamUser SteamGameServer_GetHSteamUser();
OVERRIDE bool SteamInternal_GameServer_Init( uint32_t unIP, uint16_t usPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString );

}

#endif // STEAM_GAMESERVER_H
