//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamclient012.h"
#include "isteamclient_priv.h"

namespace libtas {

static const struct ISteamClient012Vtbl ISteamClient012_vtbl = {
	ISteamClient_CreateSteamPipe,
	ISteamClient_BReleaseSteamPipe,
	ISteamClient_ConnectToGlobalUser,
	ISteamClient_CreateLocalUser,
	ISteamClient_ReleaseUser,
	ISteamClient_GetISteamUser,
	ISteamClient_GetISteamGameServer,
	ISteamClient_SetLocalIPBinding,
	ISteamClient_GetISteamFriends,
	ISteamClient_GetISteamUtils,
	ISteamClient_GetISteamMatchmaking,
	ISteamClient_GetISteamMatchmakingServers,
	ISteamClient_GetISteamGenericInterface,
	ISteamClient_GetISteamUserStats,
	ISteamClient_GetISteamGameServerStats,
	ISteamClient_GetISteamApps,
	ISteamClient_GetISteamNetworking,
	ISteamClient_GetISteamRemoteStorage,
	ISteamClient_GetISteamScreenshots,
	ISteamClient_RunFrame,
	ISteamClient_GetIPCCallCount,
	ISteamClient_SetWarningMessageHook,
	ISteamClient_BShutdownIfAllPipesClosed,
	ISteamClient_GetISteamHTTP,
	ISteamClient_GetISteamUnifiedMessages,
	ISteamClient_GetISteamController,
	ISteamClient_GetISteamUGC,
};

struct ISteamClient *SteamClient012(void)
{
	static struct ISteamClient impl;

	impl.vtbl.v012 = &ISteamClient012_vtbl;

	return &impl;
}

}
