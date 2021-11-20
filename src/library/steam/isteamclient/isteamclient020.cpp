//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamclient020.h"
#include "isteamclient_priv.h"

namespace libtas {

static const struct ISteamClient020Vtbl ISteamClient020_vtbl = {
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
    ISteamClient_GetISteamGameSearch,
	ISteamClient_RunFrame,
	ISteamClient_GetIPCCallCount,
	ISteamClient_SetWarningMessageHook,
	ISteamClient_BShutdownIfAllPipesClosed,
	ISteamClient_GetISteamHTTP,
	ISteamClient_GetISteamUnifiedMessages,
	ISteamClient_GetISteamController,
	ISteamClient_GetISteamUGC,
	ISteamClient_GetISteamAppList,
	ISteamClient_GetISteamMusic,
	ISteamClient_GetISteamMusicRemote,
	ISteamClient_GetISteamHTMLSurface,
    nullptr,
    nullptr,
    nullptr,
	ISteamClient_GetISteamInventory,
	ISteamClient_GetISteamVideo,
	ISteamClient_GetISteamParentalSettings,
    ISteamClient_GetISteamInput,
    ISteamClient_GetISteamParties,
    ISteamClient_GetISteamRemotePlay,
    nullptr,
};

struct ISteamClient *SteamClient020(void)
{
	static struct ISteamClient impl;

	impl.vtbl.v020 = &ISteamClient020_vtbl;

	return &impl;
}

}
