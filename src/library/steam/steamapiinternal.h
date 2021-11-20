//====== Copyright 1996-2015, Valve Corporation, All rights reserved. =======
//
// Purpose: Internal private Steamworks API declarations and definitions
//
//=============================================================================

#ifndef LIBTAS_STEAMAPIINTERNAL_H_INCL
#define LIBTAS_STEAMAPIINTERNAL_H_INCL

#include "../global.h"
#include "steamtypes.h"
#include "isteamuser.h"
#include "isteamuserstats.h"
#include "isteamutils.h"
#include "isteamremotestorage/isteamremotestorage.h"
#include "isteamapps.h"
#include "isteamclient/isteamclient.h"
#include "isteamfriends.h"
#include "isteamscreenshots.h"
#include "isteamugc.h"
#include "isteamcontroller.h"
#include "isteammatchmaking.h"
#include "isteamhttp.h"
#include "isteamnetworking.h"

namespace libtas {

typedef void ISteamAppList;
typedef void ISteamMusic;
typedef void ISteamMusicRemote;
typedef void ISteamHTMLSurface;
typedef void ISteamInventory;
typedef void ISteamVideo;
typedef void ISteamParentalSettings;

// CSteamAPIContext encapsulates the Steamworks API global accessors into
// a single object. This is DEPRECATED and only remains for compatibility.
class CSteamAPIContext
{
public:
	ISteamClient		*m_pSteamClient;
	ISteamUser			*m_pSteamUser;
	ISteamFriends		*m_pSteamFriends;
	ISteamUtils			*m_pSteamUtils;
	ISteamMatchmaking	*m_pSteamMatchmaking;
	ISteamUserStats		*m_pSteamUserStats;
	ISteamApps			*m_pSteamApps;
	ISteamMatchmakingServers *m_pSteamMatchmakingServers;
	ISteamNetworking	*m_pSteamNetworking;
	ISteamRemoteStorage *m_pSteamRemoteStorage;
	ISteamScreenshots	*m_pSteamScreenshots;
	ISteamHTTP			*m_pSteamHTTP;
	ISteamController	*m_pController;
	ISteamUGC			*m_pSteamUGC;
	ISteamAppList		*m_pSteamAppList;
	ISteamMusic			*m_pSteamMusic;
	ISteamMusicRemote	*m_pSteamMusicRemote;
	ISteamHTMLSurface	*m_pSteamHTMLSurface;
	ISteamInventory		*m_pSteamInventory;
	ISteamVideo			*m_pSteamVideo;
	ISteamParentalSettings *m_pSteamParentalSettings;
};

struct CSteamAPIContextInitData
{
	void (*callback)(CSteamAPIContext *ctx);
	uintptr_t ifaces_stale_cnt;
	CSteamAPIContext ctx;
};

OVERRIDE HSteamUser SteamAPI_GetHSteamUser();
OVERRIDE HSteamPipe SteamAPI_GetHSteamPipe();
OVERRIDE CSteamAPIContext* SteamInternal_ContextInit( CSteamAPIContextInitData *pContextInitData );
OVERRIDE void * SteamInternal_CreateInterface( const char *ver );
OVERRIDE void * SteamInternal_FindOrCreateUserInterface(HSteamUser steam_user, const char *version);

/* Override method CSteamAPIContext::Init() */
OVERRIDE bool _ZN16CSteamAPIContext4InitEv(CSteamAPIContext* context);

}

#endif
