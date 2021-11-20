//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCLIENT_PRIV_H_INCL
#define LIBTAS_ISTEAMCLIENT_PRIV_H_INCL


// #include "steam.h"

#include <stdint.h>
#include <string>
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

namespace libtas {

typedef void ISteamAppList;
typedef void ISteamMusic;
typedef void ISteamMusicRemote;
typedef void ISteamHTMLSurface;
typedef void ISteamInventory;
typedef void ISteamVideo;
typedef void ISteamParentalSettings;
typedef void ISteamGameServerStats;
typedef void ISteamGameSearch;
typedef void ISteamInput;
typedef void ISteamParties;
typedef void ISteamRemotePlay;

HSteamPipe ISteamClient_CreateSteamPipe(void *iface);
bool ISteamClient_BReleaseSteamPipe(void *iface, HSteamPipe steam_pipe);
HSteamUser ISteamClient_ConnectToGlobalUser(void *iface, HSteamPipe steam_pipe);
HSteamUser ISteamClient_CreateLocalUser( void *iface, HSteamPipe *phSteamPipe, EAccountType eAccountType );
void ISteamClient_ReleaseUser(void *iface, HSteamPipe steam_pipe, HSteamUser steam_user);
ISteamAppList *ISteamClient_GetISteamAppList(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamApps *ISteamClient_GetISteamApps(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamClient *ISteamClient_GetISteamClient(void *iface, HSteamPipe steam_pipe, const char *version);
ISteamController *ISteamClient_GetISteamController(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamFriends *ISteamClient_GetISteamFriends(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamGameServer *ISteamClient_GetISteamGameServer(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamGameServerStats *ISteamClient_GetISteamGameServerStats(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
void *ISteamClient_GetISteamGenericInterface(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamHTMLSurface *ISteamClient_GetISteamHTMLSurface(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamHTTP *ISteamClient_GetISteamHTTP(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamInventory *ISteamClient_GetISteamInventory(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamMatchmaking *ISteamClient_GetISteamMatchmaking(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamMatchmakingServers *ISteamClient_GetISteamMatchmakingServers(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamMusic *ISteamClient_GetISteamMusic(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamMusicRemote *ISteamClient_GetISteamMusicRemote(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamNetworking *ISteamClient_GetISteamNetworking(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamParentalSettings *ISteamClient_GetISteamParentalSettings(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamRemoteStorage *ISteamClient_GetISteamRemoteStorage(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamScreenshots *ISteamClient_GetISteamScreenshots(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamUGC *ISteamClient_GetISteamUGC(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
void *ISteamClient_GetISteamUnifiedMessages(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamUser *ISteamClient_GetISteamUser(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamUserStats *ISteamClient_GetISteamUserStats(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamUtils *ISteamClient_GetISteamUtils(void *iface, HSteamPipe steam_pipe, const char *version);
ISteamVideo *ISteamClient_GetISteamVideo(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
void ISteamClient_SetWarningMessageHook(void *iface, void *callback);
void ISteamClient_SetLocalIPBinding( void *iface, uint32_t unIP, uint16_t usPort );
void ISteamClient_RunFrame(void *iface);
unsigned int ISteamClient_GetIPCCallCount(void *iface);
bool ISteamClient_BShutdownIfAllPipesClosed(void *iface);
void ISteamClient_DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void *iface, void (*)() );
void ISteamClient_DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void *iface, void (*)() );
void ISteamClient_Set_SteamAPI_CCheckCallbackRegisteredInProcess( void *iface, SteamAPI_CheckCallbackRegistered_t func );
ISteamGameSearch *ISteamClient_GetISteamGameSearch(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamInput *ISteamClient_GetISteamInput(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamParties *ISteamClient_GetISteamParties(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);
ISteamRemotePlay *ISteamClient_GetISteamRemotePlay(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version);

}

#endif
