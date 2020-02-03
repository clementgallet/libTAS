/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../logging.h"
#include <string.h>

namespace libtas {

bool ISteamUserStats::RequestCurrentStats()
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetStat( const char *pchName, int *pData )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats::GetStat( const char *pchName, float *pData )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    if (pData)
        *pData = 0;
    return true;
}

bool ISteamUserStats::SetStat( const char *pchName, int nData )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName, " and data ", nData);
    return true;
}

bool ISteamUserStats::SetStat( const char *pchName, float fData )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName, " and data ", fData);
    return true;
}

bool ISteamUserStats::UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    return true;
}

bool ISteamUserStats::GetAchievement( const char *pchName, bool *pbAchieved )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    if (pbAchieved)
        *pbAchieved = false;
    return true;
}

bool ISteamUserStats::SetAchievement( const char *pchName )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    return true;
}

bool ISteamUserStats::ClearAchievement( const char *pchName )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    return true;
}

bool ISteamUserStats::GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    if (pbAchieved)
        *pbAchieved = false;
    if (punUnlockTime)
        *punUnlockTime = 0;
    return true;
}

bool ISteamUserStats::StoreStats()
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamUserStats::GetAchievementIcon( const char *pchName )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    return 0;
}

const char *ISteamUserStats::GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName, " and key ", pchKey);
    if (strcmp(pchKey, "hidden")) {
        return "0";
    }
    return "";
}

bool ISteamUserStats::IndicateAchievementProgress( const char *pchName, unsigned int nCurProgress, unsigned int nMaxProgress )
{
    debuglog(LCF_STEAM, __func__, " called with name ", pchName);
    return true;
}

unsigned int ISteamUserStats::GetNumAchievements()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

const char *ISteamUserStats::GetAchievementName( unsigned int iAchievement )
{
    debuglog(LCF_STEAM, __func__, " called with iAchievement ", iAchievement);
    return "";
}

SteamAPICall_t ISteamUserStats::RequestUserStats( CSteamID steamIDUser )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

bool ISteamUserStats::GetUserStat( CSteamID steamIDUser, const char *pchName, int *pData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, unsigned int *punUnlockTime )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::ResetAllStats( bool bAchievementsToo )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::FindLeaderboard( const char *pchLeaderboardName )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

const char *ISteamUserStats::GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    DEBUGLOGCALL(LCF_STEAM);
    return "";
}

int ISteamUserStats::GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

ELeaderboardSortMethod ISteamUserStats::GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

ELeaderboardDisplayType ISteamUserStats::GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUserStats::DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

bool ISteamUserStats::GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int *pDetails, int cDetailsMax )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int nScore, const int *pScoreDetails, int cScoreDetailsCount )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::GetNumberOfCurrentPlayers()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamUserStats::RequestGlobalAchievementPercentages()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

int ISteamUserStats::GetMostAchievedAchievementInfo( char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

int ISteamUserStats::GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, unsigned int unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

bool ISteamUserStats::GetAchievementAchievedPercent( const char *pchName, float *pflPercent )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamUserStats::RequestGlobalStats( int nHistoryDays )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

bool ISteamUserStats::GetGlobalStat( const char *pchStatName, int64_t *pData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamUserStats::GetGlobalStat( const char *pchStatName, double *pData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamUserStats::GetGlobalStatHistory( const char *pchStatName, int64_t *pData, unsigned int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamUserStats::GetGlobalStatHistory( const char *pchStatName, double *pData, unsigned int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

}
