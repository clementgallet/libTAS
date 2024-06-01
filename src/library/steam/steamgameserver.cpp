/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

DEFINE_ORIG_POINTER(SteamGameServer_Init)
DEFINE_ORIG_POINTER(SteamGameServer_Shutdown)
DEFINE_ORIG_POINTER(SteamGameServer_RunCallbacks)
DEFINE_ORIG_POINTER(SteamGameServer_ReleaseCurrentThreadMemory)
DEFINE_ORIG_POINTER(SteamGameServer_BSecure)
DEFINE_ORIG_POINTER(SteamGameServer_GetSteamID)
DEFINE_ORIG_POINTER(SteamGameServerClient)
DEFINE_ORIG_POINTER(SteamGameServer)
DEFINE_ORIG_POINTER(SteamGameServerUtils)
DEFINE_ORIG_POINTER(SteamGameServerNetworking)
DEFINE_ORIG_POINTER(SteamGameServerStats)
DEFINE_ORIG_POINTER(SteamGameServerHTTP)
DEFINE_ORIG_POINTER(SteamGameServerInventory)
DEFINE_ORIG_POINTER(SteamGameServerUGC)
DEFINE_ORIG_POINTER(SteamGameServerApps)
DEFINE_ORIG_POINTER(SteamGameServer_GetHSteamPipe)
DEFINE_ORIG_POINTER(SteamGameServer_GetHSteamUser)
DEFINE_ORIG_POINTER(SteamInternal_GameServer_Init)

bool SteamGameServer_Init( uint32_t unIP, uint16_t usSteamPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_Init, "steam_api");
        return orig::SteamGameServer_Init(unIP, usSteamPort, usGamePort, usQueryPort, eServerMode, pchVersionString);
    }

    return true;
}

void SteamGameServer_Shutdown()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_Shutdown, "steam_api");
        return orig::SteamGameServer_Shutdown();
    }
}

void SteamGameServer_RunCallbacks()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_RunCallbacks, "steam_api");
        return orig::SteamGameServer_RunCallbacks();
    }
}

void SteamGameServer_ReleaseCurrentThreadMemory()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_ReleaseCurrentThreadMemory, "steam_api");
        return orig::SteamGameServer_ReleaseCurrentThreadMemory();
    }
}

bool SteamGameServer_BSecure()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_BSecure, "steam_api");
        return orig::SteamGameServer_BSecure();
    }
    return false;
}

uint64_t SteamGameServer_GetSteamID()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_GetSteamID, "steam_api");
        return orig::SteamGameServer_GetSteamID();
    }
    return 0;
}

ISteamClient *SteamGameServerClient()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerClient, "steam_api");
        return orig::SteamGameServerClient();
    }

    static ISteamClient steamgameserverclient;
    return &steamgameserverclient;
}

ISteamGameServer *SteamGameServer()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer, "steam_api");
        return orig::SteamGameServer();
    }

    static ISteamGameServer steamgameserver;
    return &steamgameserver;
}

ISteamUtils *SteamGameServerUtils()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerUtils, "steam_api");
        return orig::SteamGameServerUtils();
    }

    static ISteamUtils steamgameserverutils;
    return &steamgameserverutils;
}

ISteamNetworking *SteamGameServerNetworking()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerNetworking, "steam_api");
        return orig::SteamGameServerNetworking();
    }

    static ISteamNetworking steamgameservernetworking;
    return &steamgameservernetworking;
}

ISteamGameServerStats *SteamGameServerStats()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerStats, "steam_api");
        return orig::SteamGameServerStats();
    }

    return nullptr;
}

ISteamHTTP *SteamGameServerHTTP()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerHTTP, "steam_api");
        return orig::SteamGameServerHTTP();
    }

    static ISteamHTTP steamgameserverhttp;
    return &steamgameserverhttp;
}

ISteamInventory *SteamGameServerInventory()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerInventory, "steam_api");
        return orig::SteamGameServerInventory();
    }

    return nullptr;
}

ISteamUGC *SteamGameServerUGC()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerUGC, "steam_api");
        return orig::SteamGameServerUGC();
    }

    static ISteamUGC steamgameserverugc;
    return &steamgameserverugc;
}

ISteamApps *SteamGameServerApps()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServerApps, "steam_api");
        return orig::SteamGameServerApps();
    }

    static ISteamApps steamgameserverapps;
    return &steamgameserverapps;
}

HSteamPipe SteamGameServer_GetHSteamPipe()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_GetHSteamPipe, "steam_api");
        return orig::SteamGameServer_GetHSteamPipe();
    }
    return 0;
}

HSteamUser SteamGameServer_GetHSteamUser()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamGameServer_GetHSteamUser, "steam_api");
        return orig::SteamGameServer_GetHSteamUser();
    }
    return 0;
}

bool SteamInternal_GameServer_Init( uint32_t unIP, uint16_t usPort, uint16_t usGamePort, uint16_t usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamInternal_GameServer_Init, "steam_api");
        return orig::SteamInternal_GameServer_Init(unIP, usPort, usGamePort, usQueryPort, eServerMode, pchVersionString);
    }
    return true;
}

}
