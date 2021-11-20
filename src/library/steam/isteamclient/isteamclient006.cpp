//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamclient006.h"
#include "isteamclient_priv.h"

namespace libtas {

static const struct ISteamClient006Vtbl ISteamClient006_vtbl = {
	ISteamClient_CreateSteamPipe,
	ISteamClient_BReleaseSteamPipe,
    nullptr,
	ISteamClient_ConnectToGlobalUser,
	ISteamClient_CreateLocalUser,
	ISteamClient_ReleaseUser,
	ISteamClient_GetISteamUser,
    nullptr,
	ISteamClient_GetISteamGameServer,
	nullptr,
    nullptr,
	ISteamClient_GetISteamFriends,
	ISteamClient_GetISteamUtils,
    nullptr,
	ISteamClient_GetISteamMatchmaking,
    ISteamClient_GetISteamApps,
    nullptr,
    nullptr,
	ISteamClient_GetISteamMatchmakingServers,
	ISteamClient_RunFrame,
	ISteamClient_GetIPCCallCount,
};

struct ISteamClient *SteamClient006(void)
{
	static struct ISteamClient impl;

	impl.vtbl.v006 = &ISteamClient006_vtbl;

	return &impl;
}

}
