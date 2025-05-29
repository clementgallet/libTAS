//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamuserstats013.h"
#include "isteamuserstats_priv.h"

namespace libtas {

static const struct ISteamUserStats013Vtbl ISteamUserStats013_vtbl = {
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
    ISteamUserStats_GetNumAchievements,
    ISteamUserStats_GetAchievementName,
    ISteamUserStats_RequestUserStats,
    ISteamUserStats_GetUserStatInt32,
    ISteamUserStats_GetUserStatFloat,
    ISteamUserStats_GetUserAchievement,
    ISteamUserStats_GetUserAchievementAndUnlockTime,
    ISteamUserStats_ResetAllStats,
    ISteamUserStats_FindOrCreateLeaderboard,
    ISteamUserStats_FindLeaderboard,
    ISteamUserStats_GetLeaderboardName,
    ISteamUserStats_GetLeaderboardEntryCount,
    ISteamUserStats_GetLeaderboardSortMethod,
    ISteamUserStats_GetLeaderboardDisplayType,
    ISteamUserStats_DownloadLeaderboardEntries,
    ISteamUserStats_DownloadLeaderboardEntriesForUsers,
    ISteamUserStats_GetDownloadedLeaderboardEntry,
    ISteamUserStats_UploadLeaderboardScore,
    ISteamUserStats_AttachLeaderboardUGC,
    ISteamUserStats_GetNumberOfCurrentPlayers,
    ISteamUserStats_RequestGlobalAchievementPercentages,
    ISteamUserStats_GetMostAchievedAchievementInfo,
    ISteamUserStats_GetNextMostAchievedAchievementInfo,
    ISteamUserStats_GetAchievementAchievedPercent,
    ISteamUserStats_RequestGlobalStats,
    ISteamUserStats_GetGlobalStatInt64,
    ISteamUserStats_GetGlobalStatDouble,
    ISteamUserStats_GetGlobalStatHistoryInt64,
    ISteamUserStats_GetGlobalStatHistoryDouble,
    ISteamUserStats_GetAchievementProgressLimitsInt32,
    ISteamUserStats_GetAchievementProgressLimitsFloat,
};

struct ISteamUserStats *SteamUserStats013(void)
{
    static struct ISteamUserStats impl;

    impl.vtbl.v013 = &ISteamUserStats013_vtbl;

    return &impl;
}

}
