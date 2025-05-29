//====== Copyright (c) 1996-2006, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamuserstats006.h"
#include "isteamuserstats_priv.h"

namespace libtas {

static const struct ISteamUserStats006Vtbl ISteamUserStats006_vtbl = {
    
    ISteamUserStats_RequestCurrentStats,
    ISteamUserStats_GetStatInt32,
    ISteamUserStats_GetStatFloat,
    ISteamUserStats_SetStatInt32,
    ISteamUserStats_SetStatFloat,
    ISteamUserStats_UpdateAvgRateStat,
    ISteamUserStats_GetAchievement,
    ISteamUserStats_SetAchievement,
    ISteamUserStats_ClearAchievement,
    ISteamUserStats_GetAchievementAndUnlockTime,
    ISteamUserStats_StoreStats,
    ISteamUserStats_GetAchievementIcon,
    ISteamUserStats_GetAchievementDisplayAttribute,
    ISteamUserStats_IndicateAchievementProgress,
    ISteamUserStats_RequestUserStats,
    ISteamUserStats_GetUserStatInt32,
    ISteamUserStats_GetUserStatFloat,
    ISteamUserStats_GetUserAchievement,
    ISteamUserStats_ResetAllStats,
    ISteamUserStats_FindOrCreateLeaderboard,
    ISteamUserStats_FindLeaderboard,
    ISteamUserStats_GetLeaderboardName,
    ISteamUserStats_GetLeaderboardEntryCount,
    ISteamUserStats_GetLeaderboardSortMethod,
    ISteamUserStats_GetLeaderboardDisplayType,
    ISteamUserStats_DownloadLeaderboardEntries,
    ISteamUserStats_GetDownloadedLeaderboardEntry,
    ISteamUserStats_UploadLeaderboardScore,
    ISteamUserStats_GetNumberOfCurrentPlayers,
};

struct ISteamUserStats *SteamUserStats006(void)
{
    static struct ISteamUserStats impl;

    impl.vtbl.v006 = &ISteamUserStats006_vtbl;

    return &impl;
}

}
