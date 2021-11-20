//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCLIENT012_H_INCL
#define LIBTAS_ISTEAMCLIENT012_H_INCL

#include <stdint.h>
// #include <string>
#include "../steamtypes.h"
#include "../isteamuser.h"
#include "../isteamuserstats.h"
#include "../isteamutils.h"
#include "../isteamremotestorage/isteamremotestorage.h"
#include "../isteamapps.h"
#include "../isteamfriends.h"
#include "../isteamscreenshots.h"
#include "../isteamugc.h"
#include "../isteamcontroller.h"
#include "../isteammatchmaking.h"
#include "../isteamhttp.h"
#include "../isteamgameserver.h"
#include "../isteamnetworking.h"

#include "isteamclient.h"
#define STEAMCLIENT_INTERFACE_VERSION_012 "SteamClient012"

namespace libtas {

typedef void ISteamGameServerStats;

struct ISteamClient012Vtbl
{
	HSteamPipe (*CreateSteamPipe)(void *iface);
	bool (*BReleaseSteamPipe)(void *iface, HSteamPipe steam_pipe);
	HSteamUser (*ConnectToGlobalUser)(void *iface, HSteamPipe steam_pipe);
	HSteamUser (*CreateLocalUser)(void *iface, HSteamPipe *phSteamPipe, EAccountType eAccountType );
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
};

struct ISteamClient *SteamClient012(void);

}

#endif
