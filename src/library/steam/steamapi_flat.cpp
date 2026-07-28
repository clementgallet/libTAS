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

#include "steamapi_flat.h"

#include "logging.h"
#include "hook.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

ISteamUtils *SteamAPI_SteamUtils_v010()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamUtils_v010, (), "steam_api")
    }

    static ISteamUtils steamutils;
    return &steamutils;
}

ISteamApps *SteamAPI_SteamApps_v009()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamApps_v009, (), "steam_api");
    }

    static ISteamApps steamapps;
    return &steamapps;
}

ISteamFriends *SteamAPI_SteamFriends_v018()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamFriends_v018, (), "steam_api");
    }

    static ISteamFriends steamfriends;
    return &steamfriends;
}

ISteamScreenshots *SteamAPI_SteamScreenshots_v003()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamScreenshots_v003, (), "steam_api");
    }

    static ISteamScreenshots steamscreenshots;
    return &steamscreenshots;
}

ISteamUGC *SteamAPI_SteamUGC_v021()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamUGC_v021, (), "steam_api");
    }

    static ISteamUGC steamugc;
    return &steamugc;
}

ISteamMatchmaking *SteamAPI_SteamMatchmaking_v009()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamMatchmaking_v009, (), "steam_api");
    }

    static ISteamMatchmaking steammatchmaking;
    return &steammatchmaking;
}

ISteamNetworking *SteamAPI_SteamNetworking_v006()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamNetworking_v006, (), "steam_api");
    }

    static ISteamNetworking steamnetworking;
    return &steamnetworking;
}

ISteamMatchmakingServers *SteamAPI_SteamMatchmakingServers_v002()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamMatchmakingServers_v002, (), "steam_api");
    }

    static ISteamMatchmakingServers steammatchmakingservers;
    return &steammatchmakingservers;
}

ISteamHTTP *SteamAPI_SteamHTTP_v003()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamHTTP_v003, (), "steam_api");
    }

    static ISteamHTTP steamhttp;
    return &steamhttp;
}

ISteamMusic *SteamAPI_SteamMusic_v001()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamMusic_v001, (), "steam_api");
    }

    return nullptr;
}

ISteamHTMLSurface *SteamAPI_SteamHTMLSurface_v005()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamHTMLSurface_v005, (), "steam_api");
    }

    return nullptr;
}

ISteamInventory *SteamAPI_SteamInventory_v003()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamInventory_v003, (), "steam_api");
    }

    return nullptr;
}

ISteamVideo *SteamAPI_SteamVideo_v007()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamVideo_v007, (), "steam_api");
    }

    return nullptr;
}

ISteamParentalSettings *SteamAPI_SteamParentalSettings_v001()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamParentalSettings_v001, (), "steam_api");
    }

    return nullptr;
}

ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v004()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamNetworkingUtils_SteamAPI_v004, (), "steam_api");
    }

    static ISteamNetworkingUtils steamnetworkingutils;
    return &steamnetworkingutils;
}

ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v012()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamNetworkingSockets_SteamAPI_v012, (), "steam_api");
    }

    static ISteamNetworkingSockets steamnetworkingsockets;
    return &steamnetworkingsockets;
}

ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_SteamAPI_v002()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamNetworkingMessages_SteamAPI_v002, (), "steam_api");
    }

    static ISteamNetworkingMessages steamnetworkingmessages;
    return &steamnetworkingmessages;
}

ISteamInput *SteamAPI_SteamInput_v006()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamInput_v006, (), "steam_api");
    }

    static ISteamInput steaminput;
    return &steaminput;
}

ISteamRemotePlay *SteamAPI_SteamRemotePlay_v004()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAPI_SteamRemotePlay_v004, (), "steam_api");
    }

    return nullptr;
}

}
