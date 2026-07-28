/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "steamgameserver.h"

#include "logging.h"
#include "hook.h"
#include "global.h"

namespace libtas {

bool SteamGameServer_Init( uint32_t unIP, uint16_t usSteamPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_Init, (unIP, usSteamPort, usGamePort, usQueryPort, eServerMode, pchVersionString), "steam_api");
    }

    return true;
}

void SteamGameServer_Shutdown()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_Shutdown, (), "steam_api");
    }
}

void SteamGameServer_RunCallbacks()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_RunCallbacks, (), "steam_api");
    }
}

void SteamGameServer_ReleaseCurrentThreadMemory()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_ReleaseCurrentThreadMemory, (), "steam_api");
    }
}

bool SteamGameServer_BSecure()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_BSecure, (), "steam_api");
    }
    return false;
}

uint64_t SteamGameServer_GetSteamID()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_GetSteamID, (), "steam_api");
    }
    return 0;
}

ISteamClient *SteamGameServerClient()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerClient, (), "steam_api");
    }

    static ISteamClient steamgameserverclient;
    return &steamgameserverclient;
}

ISteamGameServer *SteamGameServer()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer, (), "steam_api");
    }

    static ISteamGameServer steamgameserver;
    return &steamgameserver;
}

ISteamUtils *SteamGameServerUtils()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerUtils, (), "steam_api");
    }

    static ISteamUtils steamgameserverutils;
    return &steamgameserverutils;
}

ISteamNetworking *SteamGameServerNetworking()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerNetworking, (), "steam_api");
    }

    static ISteamNetworking steamgameservernetworking;
    return &steamgameservernetworking;
}

ISteamGameServerStats *SteamGameServerStats()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerStats, (), "steam_api");
    }

    return nullptr;
}

ISteamHTTP *SteamGameServerHTTP()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerHTTP, (), "steam_api");
    }

    static ISteamHTTP steamgameserverhttp;
    return &steamgameserverhttp;
}

ISteamInventory *SteamGameServerInventory()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerInventory, (), "steam_api");
    }

    return nullptr;
}

ISteamUGC *SteamGameServerUGC()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerUGC, (), "steam_api");
    }

    static ISteamUGC steamgameserverugc;
    return &steamgameserverugc;
}

ISteamApps *SteamGameServerApps()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServerApps, (), "steam_api");
    }

    static ISteamApps steamgameserverapps;
    return &steamgameserverapps;
}

HSteamPipe SteamGameServer_GetHSteamPipe()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_GetHSteamPipe, (), "steam_api");
    }
    return 0;
}

HSteamUser SteamGameServer_GetHSteamUser()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamGameServer_GetHSteamUser, (), "steam_api");
    }
    return 0;
}

bool SteamInternal_GameServer_Init( uint32_t unIP, uint16_t usPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamInternal_GameServer_Init, (unIP, usPort, usGamePort, usQueryPort, eServerMode, pchVersionString), "steam_api");
    }
    return true;
}

}
