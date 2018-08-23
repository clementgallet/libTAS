/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

}
