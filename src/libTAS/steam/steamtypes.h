//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_STEAMTYPES_H_INCL
#define LIBTAS_STEAMTYPES_H_INCL

#include <stdint.h>

namespace libtas {

typedef int HSteamUser;
typedef unsigned int HAuthTicket;

typedef uint64_t CSteamID;
typedef uint64_t CGameID;

typedef unsigned int AppId_t;
typedef uint64_t SteamAPICall_t;

typedef int EVoiceResult;
typedef int EBeginAuthSessionResult;
typedef int EUserHasLicenseForAppResult;
typedef int ELeaderboardSortMethod;
typedef int ELeaderboardDisplayType;
typedef int ELeaderboardDataRequest;
}

#endif
