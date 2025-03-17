//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamuser021.h"
#include "isteamuser_priv.h"

namespace libtas {

static const struct ISteamUser021Vtbl ISteamUser021_vtbl = {
    
    ISteamUser_GetHSteamUser,
    ISteamUser_BLoggedOn,
    ISteamUser_GetSteamID,
    ISteamUser_InitiateGameConnection,
    ISteamUser_TerminateGameConnection,
    ISteamUser_TrackAppUsageEvent,
    ISteamUser_GetUserDataFolder,
    ISteamUser_StartVoiceRecording,
    ISteamUser_StopVoiceRecording,
    ISteamUser_GetAvailableVoice,
    ISteamUser_GetVoice,
    ISteamUser_DecompressVoice,
    ISteamUser_GetVoiceOptimalSampleRate,
    ISteamUser_GetAuthSessionTicket,
    ISteamUser_BeginAuthSession,
    ISteamUser_EndAuthSession,
    ISteamUser_CancelAuthTicket,
};

struct ISteamUser *SteamUser021(void)
{
	static struct ISteamUser impl;

	impl.vtbl.v021 = &ISteamUser021_vtbl;

	return &impl;
}

}
