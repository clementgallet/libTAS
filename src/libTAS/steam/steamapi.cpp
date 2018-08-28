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

#include "steamapi.h"
#include "../logging.h"
#include <signal.h>

namespace libtas {

bool SteamAPI_Init()
{
    debuglog(LCF_STEAM, __func__, " call.");
	return true;
}

bool SteamAPI_InitSafe()
{
    debuglog(LCF_STEAM, __func__, " call.");
	return true;
}

void SteamAPI_Shutdown()
{
    debuglog(LCF_STEAM, __func__, " call.");
    return;
}

bool SteamAPI_IsSteamRunning()
{
    debuglog(LCF_STEAM, __func__, " call.");
	return true;
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    debuglog(LCF_STEAM, __func__, " call.");
	return false;
}

void SteamAPI_RunCallbacks()
{
    debuglog(LCF_STEAM, __func__, " call.");
}

void SteamAPI_RegisterCallback( void *pCallback, int iCallback )
{
    debuglog(LCF_STEAM, __func__, " call.");
}

void SteamAPI_UnregisterCallback( void *pCallback )
{
    debuglog(LCF_STEAM, __func__, " call.");
}

ISteamClient *SteamClient()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamClient steamclient;
    return &steamclient;
}

ISteamUserStats *SteamUserStats()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamUserStats steamuserstats;
    return &steamuserstats;
}

ISteamUser *SteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamUser steamuser;
    return &steamuser;
}

ISteamUtils *SteamUtils()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamUtils steamutils;
    return &steamutils;
}

ISteamRemoteStorage *SteamRemoteStorage()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamRemoteStorage steamremotestorage;
    return &steamremotestorage;
}

ISteamApps *SteamApps()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamApps steamapps;
    return &steamapps;
}

ISteamFriends *SteamFriends()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamFriends steamfriends;
    return &steamfriends;
}

ISteamScreenshots *SteamScreenshots()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamScreenshots steamscreenshots;
    return &steamscreenshots;
}

ISteamUGC *SteamUGC()
{
    DEBUGLOGCALL(LCF_STEAM);

    static ISteamUGC steamugc;
    return &steamugc;
}

}
