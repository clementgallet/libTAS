/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "isteamuserstats.h"
#include "isteamuserstats_priv.h"
#include "isteamuserstats005.h"
#include "isteamuserstats006.h"
#include "isteamuserstats007.h"
#include "isteamuserstats008.h"
#include "isteamuserstats010.h"
#include "isteamuserstats011.h"
#include "isteamuserstats012.h"
#include "isteamuserstats013.h"

#include "steam/CCallback.h"
#include "steam/CCallbackManager.h"

#include "logging.h"
#include "hook.h"
#include "Utils.h"
#include "global.h"

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h> 
#include <string.h>

namespace libtas {

DEFINE_ORIG_POINTER(SteamUserStats)

static const char *steamuserstats_version = NULL;

struct ISteamUserStats *SteamUserStats_generic(const char *version)
{
    static const struct
    {
        const char *name;
        struct ISteamUserStats *(*iface_getter)(void);
    } ifaces[] = {
        { STEAMUSERSTATS_INTERFACE_VERSION_003, SteamUserStats005 },
        { STEAMUSERSTATS_INTERFACE_VERSION_004, SteamUserStats005 },
        { STEAMUSERSTATS_INTERFACE_VERSION_005, SteamUserStats005 },
        { STEAMUSERSTATS_INTERFACE_VERSION_006, SteamUserStats006 },
        { STEAMUSERSTATS_INTERFACE_VERSION_007, SteamUserStats007 },
        { STEAMUSERSTATS_INTERFACE_VERSION_008, SteamUserStats008 },
        { STEAMUSERSTATS_INTERFACE_VERSION_009, SteamUserStats010 },
        { STEAMUSERSTATS_INTERFACE_VERSION_010, SteamUserStats010 },
        { STEAMUSERSTATS_INTERFACE_VERSION_011, SteamUserStats011 },
        { STEAMUSERSTATS_INTERFACE_VERSION_012, SteamUserStats012 },
        { STEAMUSERSTATS_INTERFACE_VERSION_013, SteamUserStats013 },
        { NULL, NULL }
    };
    int i;

    LOG(LL_DEBUG, LCF_STEAM, "%s called with version %s", __func__, version);

    i = 0;
    while (ifaces[i].name)
    {
        if (strcmp(ifaces[i].name, version) == 0)
        {
            if (ifaces[i].iface_getter)
                return ifaces[i].iface_getter();

            break;
        }
        i++;
    }

    LOG(LL_WARN, LCF_STEAM, "Unable to find ISteamUserStats version %s", version);

    return nullptr;
}

void SteamUserStats_set_version(const char *version)
{
    LOG(LL_DEBUG, LCF_STEAM, "%s called with version %s", __func__, version);

    if (!steamuserstats_version)
        steamuserstats_version = version;
}

struct ISteamUserStats *SteamUserStats(void)
{
    LOGTRACE(LCF_STEAM);

    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUserStats, "steam_api");
        return orig::SteamUserStats();
    }

    static struct ISteamUserStats *cached_iface = nullptr;

    if (!steamuserstats_version)
    {
        steamuserstats_version = STEAMUSERSTATS_INTERFACE_VERSION_013;
        LOG(LL_WARN, LCF_STEAM, "ISteamUserStats: No version specified, defaulting to %s", steamuserstats_version);
    }

    if (!cached_iface)
        cached_iface = SteamUserStats_generic(steamuserstats_version);

    return cached_iface;
}

bool ISteamUserStats_RequestCurrentStats()
{
    LOGTRACE(LCF_STEAM);

    struct UserStatsReceived_t user_stats_received;
    user_stats_received.m_nGameID = 105600; // TODO
    user_stats_received.m_eResult = 1; // k_EResultOK
    user_stats_received.m_steamIDUser = 1; // TODO
    
    CCallbackManager::DispatchCallbackOutput(STEAM_CALLBACK_TYPE_USER_STATS_USER_STATS_RECEIVED, &user_stats_received, sizeof(user_stats_received));
    
    return true;
}

bool ISteamUserStats_GetStatInt32( const char *pchName, int *pData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats_GetStatFloat( const char *pchName, float *pData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats_SetStatInt32( const char *pchName, int nData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and data %d", __func__, pchName, nData);
    return true;
}

bool ISteamUserStats_SetStatFloat( const char *pchName, float fData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and data %f", __func__, pchName, fData);
    return true;
}

bool ISteamUserStats_UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats_GetAchievement( const char *pchName, bool *pbAchieved )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pbAchieved)
        *pbAchieved = false;
    return true;
}

bool ISteamUserStats_SetAchievement( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats_ClearAchievement( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats_GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pbAchieved)
        *pbAchieved = false;
    if (punUnlockTime)
        *punUnlockTime = 0;
    return true;
}

bool ISteamUserStats_StoreStats()
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamUserStats_GetAchievementIcon( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return 0;
}

const char *ISteamUserStats_GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and key %s", __func__, pchName, pchKey);
    if (strcmp(pchKey, "hidden")) {
        return "0";
    }
    return "";
}

bool ISteamUserStats_IndicateAchievementProgress( const char *pchName, unsigned int nCurProgress, unsigned int nMaxProgress )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

unsigned int ISteamUserStats_GetNumAchievements()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

const char *ISteamUserStats_GetAchievementName( unsigned int iAchievement )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with iAchievement %d", __func__, iAchievement);
    return "";
}

SteamAPICall_t ISteamUserStats_RequestUserStats( CSteamID steamIDUser )
{
    LOGTRACE(LCF_STEAM);

    SteamAPICall_t api_call = CCallbackManager::AwaitApiCallResultOutput();

    struct UserStatsReceived_t user_stats_received;
    bool io_failure = false;

    user_stats_received.m_nGameID = 105600; // TODO
    user_stats_received.m_eResult = 1; // k_EResultOK
    user_stats_received.m_steamIDUser = steamIDUser; // TODO
    
    CCallbackManager::DispatchApiCallResultOutput(api_call, STEAM_CALLBACK_TYPE_USER_STATS_USER_STATS_RECEIVED, io_failure, &user_stats_received, sizeof(user_stats_received));
    
    return api_call;
}

bool ISteamUserStats_GetUserStatInt32( CSteamID steamIDUser, const char *pchName, int *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_GetUserStatFloat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_ResetAllStats( bool bAchievementsToo )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats_FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats_FindLeaderboard( const char *pchLeaderboardName )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

const char *ISteamUserStats_GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

int ISteamUserStats_GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ELeaderboardSortMethod ISteamUserStats_GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ELeaderboardDisplayType ISteamUserStats_GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUserStats_DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats_DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

bool ISteamUserStats_GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int *pDetails, int cDetailsMax )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats_UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int nScore, const int *pScoreDetails, int cScoreDetailsCount )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats_AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats_GetNumberOfCurrentPlayers()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats_RequestGlobalAchievementPercentages()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

int ISteamUserStats_GetMostAchievedAchievementInfo( char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

int ISteamUserStats_GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

bool ISteamUserStats_GetAchievementAchievedPercent( const char *pchName, float *pflPercent )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats_RequestGlobalStats( int nHistoryDays )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

bool ISteamUserStats_GetGlobalStatInt64( const char *pchStatName, int64_t *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_GetGlobalStatDouble( const char *pchStatName, double *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamUserStats_GetGlobalStatHistoryInt64( const char *pchStatName, int64_t *pData, unsigned int cubData )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamUserStats_GetGlobalStatHistoryDouble( const char *pchStatName, double *pData, unsigned int cubData )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamUserStats_GetAchievementProgressLimitsInt32( const char *pchName, int32_t *pnMinProgress, int32_t *pnMaxProgress )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats_GetAchievementProgressLimitsFloat( const char *pchName, float *pfMinProgress, float *pfMaxProgress )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

}
