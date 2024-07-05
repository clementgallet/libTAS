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
#include "CCallback.h"
#include "CCallbackManager.h"

#include "logging.h"

#include <string.h>

namespace libtas {

bool ISteamUserStats::RequestCurrentStats()
{
    LOGTRACE(LCF_STEAM);

    struct UserStatsReceived_t user_stats_received;
    user_stats_received.m_nGameID = 105600; // TODO
    user_stats_received.m_eResult = 1; // k_EResultOK
    user_stats_received.m_steamIDUser = 1; // TODO
    
    CCallbackManager::DispatchCallbackOutput(STEAM_CALLBACK_TYPE_USER_STATS_USER_STATS_RECEIVED, &user_stats_received, sizeof(user_stats_received));
    
    return true;
}

bool ISteamUserStats::GetStat( const char *pchName, int *pData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats::GetStat( const char *pchName, float *pData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats::SetStat( const char *pchName, int nData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and data %d", __func__, pchName, nData);
    return true;
}

bool ISteamUserStats::SetStat( const char *pchName, float fData )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and data %f", __func__, pchName, fData);
    return true;
}

bool ISteamUserStats::UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats::GetAchievement( const char *pchName, bool *pbAchieved )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pbAchieved)
        *pbAchieved = false;
    return true;
}

bool ISteamUserStats::SetAchievement( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats::ClearAchievement( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

bool ISteamUserStats::GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    if (pbAchieved)
        *pbAchieved = false;
    if (punUnlockTime)
        *punUnlockTime = 0;
    return true;
}

bool ISteamUserStats::StoreStats()
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamUserStats::GetAchievementIcon( const char *pchName )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return 0;
}

const char *ISteamUserStats::GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s and key %s", __func__, pchName, pchKey);
    if (strcmp(pchKey, "hidden")) {
        return "0";
    }
    return "";
}

bool ISteamUserStats::IndicateAchievementProgress( const char *pchName, unsigned int nCurProgress, unsigned int nMaxProgress )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with name %s", __func__, pchName);
    return true;
}

unsigned int ISteamUserStats::GetNumAchievements()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

const char *ISteamUserStats::GetAchievementName( unsigned int iAchievement )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with iAchievement %d", __func__, iAchievement);
    return "";
}

SteamAPICall_t ISteamUserStats::RequestUserStats( CSteamID steamIDUser )
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

bool ISteamUserStats::GetUserStat( CSteamID steamIDUser, const char *pchName, int *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats::ResetAllStats( bool bAchievementsToo )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::FindLeaderboard( const char *pchLeaderboardName )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

const char *ISteamUserStats::GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

int ISteamUserStats::GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ELeaderboardSortMethod ISteamUserStats::GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ELeaderboardDisplayType ISteamUserStats::GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUserStats::DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

bool ISteamUserStats::GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int *pDetails, int cDetailsMax )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int nScore, const int *pScoreDetails, int cScoreDetailsCount )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::GetNumberOfCurrentPlayers()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::RequestGlobalAchievementPercentages()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

int ISteamUserStats::GetMostAchievedAchievementInfo( char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

int ISteamUserStats::GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

bool ISteamUserStats::GetAchievementAchievedPercent( const char *pchName, float *pflPercent )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::RequestGlobalStats( int nHistoryDays )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

bool ISteamUserStats::GetGlobalStat( const char *pchStatName, int64_t *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetGlobalStat( const char *pchStatName, double *pData )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamUserStats::GetGlobalStatHistory( const char *pchStatName, int64_t *pData, unsigned int cubData )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamUserStats::GetGlobalStatHistory( const char *pchStatName, double *pData, unsigned int cubData )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

}
