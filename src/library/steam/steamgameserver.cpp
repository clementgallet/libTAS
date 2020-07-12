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

#include "steamapi.h"
#include "../logging.h"
#include <signal.h>
// #define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>

namespace libtas {

bool SteamGameServer_Init( uint32_t unIP, uint16_t usSteamPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void SteamGameServer_Shutdown()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void SteamGameServer_RunCallbacks()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void SteamGameServer_ReleaseCurrentThreadMemory()
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool SteamGameServer_BSecure()
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

uint64_t SteamGameServer_GetSteamID()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

ISteamClient *SteamGameServerClient()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamClient steamgameserverclient;
    return &steamgameserverclient;
}

ISteamGameServer *SteamGameServer()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamGameServer steamgameserver;
    return &steamgameserver;
}

ISteamUtils *SteamGameServerUtils()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUtils steamgameserverutils;
    return &steamgameserverutils;
}

ISteamNetworking *SteamGameServerNetworking()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamGameServerStats *SteamGameServerStats()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamHTTP *SteamGameServerHTTP()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamHTTP steamgameserverhttp;
    return &steamgameserverhttp;
}

ISteamInventory *SteamGameServerInventory()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamUGC *SteamGameServerUGC()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUGC steamgameserverugc;
    return &steamgameserverugc;
}

ISteamApps *SteamGameServerApps()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamApps steamgameserverapps;
    return &steamgameserverapps;
}

HSteamPipe SteamGameServer_GetHSteamPipe()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamUser SteamGameServer_GetHSteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool SteamInternal_GameServer_Init( uint32_t unIP, uint16_t usPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

}
