//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCLIENT014_H_INCL
#define LIBTAS_ISTEAMCLIENT014_H_INCL

#include "isteamclient.h"
#include "steam/steamtypes.h"
#include "steam/isteamuser/isteamuser.h"
#include "steam/isteamuserstats/isteamuserstats.h"
#include "steam/isteamutils.h"
#include "steam/isteamremotestorage/isteamremotestorage.h"
#include "steam/isteamapps.h"
#include "steam/isteamfriends.h"
#include "steam/isteamscreenshots.h"
#include "steam/isteamugc.h"
#include "steam/isteamcontroller.h"
#include "steam/isteammatchmaking.h"
#include "steam/isteamhttp.h"
#include "steam/isteamgameserver.h"
#include "steam/isteamnetworking.h"

#include <stdint.h>
#define STEAMCLIENT_INTERFACE_VERSION_014 "SteamClient014"

namespace libtas {

typedef void ISteamAppList;
typedef void ISteamMusic;
typedef void ISteamGameServerStats;

struct ISteamClient014Vtbl
{
	HSteamPipe (*CreateSteamPipe)(void *iface);
	bool (*BReleaseSteamPipe)(void *iface, HSteamPipe steam_pipe);
	HSteamUser (*ConnectToGlobalUser)(void *iface, HSteamPipe steam_pipe);
	HSteamUser (*CreateLocalUser)( void *iface, HSteamPipe *phSteamPipe, EAccountType eAccountType );
	void (*ReleaseUser)(void *iface, HSteamPipe steam_pipe, HSteamUser steam_user);
	ISteamUser *(*GetISteamUser)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamGameServer *(*GetISteamGameServer)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
    void (*SetLocalIPBinding)( void *iface, uint32_t unIP, uint16_t usPort );
	ISteamFriends *(*GetISteamFriends)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamUtils *(*GetISteamUtils)(void *iface, HSteamPipe steam_pipe, const char *version);
	ISteamMatchmaking *(*GetISteamMatchmaking)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamMatchmakingServers *(*GetISteamMatchmakingServers)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	void *(*GetISteamGenericInterface)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamUserStats *(*GetISteamUserStats)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamGameServerStats *(*GetISteamGameServerStats)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamApps *(*GetISteamApps)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamNetworking *(*GetISteamNetworking)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamRemoteStorage *(*GetISteamRemoteStorage)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamScreenshots *(*GetISteamScreenshots)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
    void (*RunFrame)(void *iface);
	unsigned int (*GetIPCCallCount)(void *iface);
	void (*SetWarningMessageHook)(void *iface, void *callback);
	bool (*BShutdownIfAllPipesClosed)(void *iface);
	ISteamHTTP *(*GetISteamHTTP)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	void *(*GetISteamUnifiedMessages)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamController *(*GetISteamController)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamUGC *(*GetISteamUGC)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamAppList *(*GetISteamAppList)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamMusic *(*GetISteamMusic)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
};

struct ISteamClient *SteamClient014(void);

}

#endif
