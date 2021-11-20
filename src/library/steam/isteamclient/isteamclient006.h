//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCLIENT006_H_INCL
#define LIBTAS_ISTEAMCLIENT006_H_INCL

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
#define STEAMCLIENT_INTERFACE_VERSION_006 "SteamClient006"

namespace libtas {

struct ISteamClient006Vtbl
{
	HSteamPipe (*CreateSteamPipe)(void *iface);
	bool (*BReleaseSteamPipe)(void *iface, HSteamPipe steam_pipe);
    void *CreateGlobalUser;
	HSteamUser (*ConnectToGlobalUser)(void *iface, HSteamPipe steam_pipe);
	HSteamUser (*CreateLocalUser)( void *iface, HSteamPipe *phSteamPipe, EAccountType eAccountType );
	void (*ReleaseUser)(void *iface, HSteamPipe steam_pipe, HSteamUser steam_user);
	ISteamUser *(*GetISteamUser)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
    void *GetIVAC;
	ISteamGameServer *(*GetISteamGameServer)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	void *SetLocalIPBinding;
    void *GetUniverseName;
	ISteamFriends *(*GetISteamFriends)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	ISteamUtils *(*GetISteamUtils)(void *iface, HSteamPipe steam_pipe, const char *version);
    void *GetISteamBilling;
	ISteamMatchmaking *(*GetISteamMatchmaking)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
    ISteamApps *(*GetISteamApps)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
    void *GetISteamContentServer;
    void *GetISteamMasterServerUpdater;
	ISteamMatchmakingServers *(*GetISteamMatchmakingServers)(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
	void (*RunFrame)(void *iface);
	unsigned int (*GetIPCCallCount)(void *iface);
};

struct ISteamClient *SteamClient006(void);

}

#endif
