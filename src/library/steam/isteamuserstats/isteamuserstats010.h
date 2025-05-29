//====== Copyright ))(c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSERSTATS010_H_INCL
#define LIBTAS_ISTEAMUSERSTATS010_H_INCL

#include "isteamuserstats.h"

#include <stdint.h>
#define STEAMUSERSTATS_INTERFACE_VERSION_009 "STEAMUSERSTATS_INTERFACE_VERSION009"
#define STEAMUSERSTATS_INTERFACE_VERSION_010 "STEAMUSERSTATS_INTERFACE_VERSION010"

namespace libtas {

struct ISteamUserStats010Vtbl
{
    bool (*RequestCurrentStats)();
    bool (*GetStatInt32)( const char *pchName, int *pData );
    bool (*GetStatFloat)( const char *pchName, float *pData );
    bool (*SetStatInt32)( const char *pchName, int nData );
    bool (*SetStatFloat)( const char *pchName, float fData );
    bool (*UpdateAvgRateStat)( const char *pchName, float flCountThisSession, double dSessionLength );
    bool (*GetAchievement)( const char *pchName, bool *pbAchieved );
    bool (*SetAchievement)( const char *pchName );
    bool (*ClearAchievement)( const char *pchName );
    bool (*GetAchievementAndUnlockTime)( const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime );
    bool (*StoreStats)();
    int (*GetAchievementIcon)( const char *pchName );
    const char *(*GetAchievementDisplayAttribute)( const char *pchName, const char *pchKey );
    bool (*IndicateAchievementProgress)( const char *pchName, unsigned int nCurProgress, unsigned int nMaxProgress );
    SteamAPICall_t (*RequestUserStats)( CSteamID steamIDUser );
    bool (*GetUserStatInt32)( CSteamID steamIDUser, const char *pchName, int *pData );
    bool (*GetUserStatFloat)( CSteamID steamIDUser, const char *pchName, float *pData );
    bool (*GetUserAchievement)( CSteamID steamIDUser, const char *pchName, bool *pbAchieved );
    bool (*GetUserAchievementAndUnlockTime)( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime );
    bool (*ResetAllStats)( bool bAchievementsToo );
    SteamAPICall_t (*FindOrCreateLeaderboard)( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType );
    SteamAPICall_t (*FindLeaderboard)( const char *pchLeaderboardName );
    const char *(*GetLeaderboardName)( SteamLeaderboard_t hSteamLeaderboard );
    int (*GetLeaderboardEntryCount)( SteamLeaderboard_t hSteamLeaderboard );
    ELeaderboardSortMethod (*GetLeaderboardSortMethod)( SteamLeaderboard_t hSteamLeaderboard );
    ELeaderboardDisplayType (*GetLeaderboardDisplayType)( SteamLeaderboard_t hSteamLeaderboard );
    SteamAPICall_t (*DownloadLeaderboardEntries)( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd );
    SteamAPICall_t (*DownloadLeaderboardEntriesForUsers)( SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers );
    bool (*GetDownloadedLeaderboardEntry)( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int *pDetails, int cDetailsMax );
    SteamAPICall_t (*UploadLeaderboardScore)( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int nScore, const int *pScoreDetails, int cScoreDetailsCount );
    SteamAPICall_t (*AttachLeaderboardUGC)( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC );
    SteamAPICall_t (*GetNumberOfCurrentPlayers)();
    SteamAPICall_t (*RequestGlobalAchievementPercentages)();
    int (*GetMostAchievedAchievementInfo)( char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved );
    int (*GetNextMostAchievedAchievementInfo)( int iIteratorPrevious, char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved );
    bool (*GetAchievementAchievedPercent)( const char *pchName, float *pflPercent );
    SteamAPICall_t (*RequestGlobalStats)( int nHistoryDays );
    bool (*GetGlobalStatInt64)( const char *pchStatName, int64_t *pData );
    bool (*GetGlobalStatDouble)( const char *pchStatName, double *pData );
    int (*GetGlobalStatHistoryInt64)( const char *pchStatName, int64_t *pData, unsigned int cubData );
    int (*GetGlobalStatHistoryDouble)( const char *pchStatName, double *pData, unsigned int cubData );
};

struct ISteamUserStats *SteamUserStats010(void);

}

#endif
