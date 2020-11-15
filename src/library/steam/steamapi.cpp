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

#include "CCallbackManager.h"
#include "isteamremotestorage/isteamremotestorage001.h"
#include "isteamremotestorage/isteamremotestorage012.h"
#include "isteamremotestorage/isteamremotestorage013.h"
#include "isteamremotestorage/isteamremotestorage014.h"

namespace libtas {

static bool SteamGetInterfaceVersion()
{
    static const struct
    {
        const char *name;
        void (*iface_set_default_version)(const char *);
    } ifaces[] = {
        // { STEAMAPPLIST_INTERFACE_VERSION_001, SteamAppList_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_001, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_003, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_005, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_006, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_007, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_008, SteamApps_set_version },
        // { STEAMCLIENT_INTERFACE_VERSION_006, SteamClient_set_version },
        // { STEAMCLIENT_INTERFACE_VERSION_012, SteamClient_set_version },
        // { STEAMCLIENT_INTERFACE_VERSION_014, SteamClient_set_version },
        // { STEAMCLIENT_INTERFACE_VERSION_016, SteamClient_set_version },
        // { STEAMCLIENT_INTERFACE_VERSION_017, SteamClient_set_version },
        // { STEAMCONTROLLER_INTERFACE_VERSION_001, SteamController_set_version },
        // { STEAMCONTROLLER_INTERFACE_VERSION_003, SteamController_set_version },
        // { STEAMCONTROLLER_INTERFACE_VERSION_005, SteamController_set_version },
        // { STEAMCONTROLLER_INTERFACE_VERSION_006, SteamController_set_version },
        // { STEAMFRIENDS_INTERFACE_VERSION_001, SteamFriends_set_version },
        // { STEAMFRIENDS_INTERFACE_VERSION_013, SteamFriends_set_version },
        // { STEAMFRIENDS_INTERFACE_VERSION_014, SteamFriends_set_version },
        // { STEAMFRIENDS_INTERFACE_VERSION_015, SteamFriends_set_version },
        // { STEAMGAMECOORDINATOR_INTERFACE_VERSION_001, SteamGameCoordinator_set_version },
        // { STEAMGAMESERVER_INTERFACE_VERSION_011, SteamGameServer_set_version },
        // { STEAMGAMESERVER_INTERFACE_VERSION_012, SteamGameServer_set_version },
        // { STEAMGAMESERVERSTATS_INTERFACE_VERSION_001, SteamGameServerStats_set_version },
        // { STEAMHTMLSURFACE_INTERFACE_VERSION_002, SteamHTMLSurface_set_version },
        // { STEAMHTMLSURFACE_INTERFACE_VERSION_003, SteamHTMLSurface_set_version },
        // { STEAMHTMLSURFACE_INTERFACE_VERSION_004, SteamHTMLSurface_set_version },
        // { STEAMHTTP_INTERFACE_VERSION_001, SteamHTTP_set_version },
        // { STEAMHTTP_INTERFACE_VERSION_002, SteamHTTP_set_version },
        // { STEAMINVENTORY_INTERFACE_VERSION_001, SteamInventory_set_version },
        // { STEAMINVENTORY_INTERFACE_VERSION_002, SteamInventory_set_version },
        // { STEAMMATCHMAKING_INTERFACE_VERSION_001, SteamMatchmaking_set_version },
        // { STEAMMATCHMAKING_INTERFACE_VERSION_009, SteamMatchmaking_set_version },
        // { STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION_001, SteamMatchmakingServers_set_version },
        // { STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION_002, SteamMatchmakingServers_set_version },
        // { STEAMMUSIC_INTERFACE_VERSION_001, SteamMusic_set_version },
        // { STEAMMUSICREMOTE_INTERFACE_VERSION_001, SteamMusicRemote_set_version },
        // { STEAMNETWORKING_INTERFACE_VERSION_001, SteamNetworking_set_version },
        // { STEAMNETWORKING_INTERFACE_VERSION_005, SteamNetworking_set_version },
        // { STEAMPARENTALSETTINGS_INTERFACE_VERSION_001, SteamParentalSettings_set_version },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_001, SteamRemoteStorage_set_version },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_012, SteamRemoteStorage_set_version },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_013, SteamRemoteStorage_set_version },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_014, SteamRemoteStorage_set_version },
        // { STEAMSCREENSHOTS_INTERFACE_VERSION_001, SteamScreenshots_set_version },
        // { STEAMSCREENSHOTS_INTERFACE_VERSION_002, SteamScreenshots_set_version },
        // { STEAMSCREENSHOTS_INTERFACE_VERSION_003, SteamScreenshots_set_version },
        // { STEAMUGC_INTERFACE_VERSION_001, SteamUGC_set_version },
        // { STEAMUGC_INTERFACE_VERSION_005, SteamUGC_set_version },
        // { STEAMUGC_INTERFACE_VERSION_007, SteamUGC_set_version },
        // { STEAMUGC_INTERFACE_VERSION_009, SteamUGC_set_version },
        // { STEAMUGC_INTERFACE_VERSION_010, SteamUGC_set_version },
        // { STEAMUNIFIEDMESSAGES_INTERFACE_VERSION_001, SteamUnifiedMessages_set_version },
        // { STEAMUSER_INTERFACE_VERSION_004, SteamUser_set_version },
        // { STEAMUSER_INTERFACE_VERSION_016, SteamUser_set_version },
        // { STEAMUSER_INTERFACE_VERSION_017, SteamUser_set_version },
        // { STEAMUSER_INTERFACE_VERSION_018, SteamUser_set_version },
        // { STEAMUSER_INTERFACE_VERSION_019, SteamUser_set_version },
        // { STEAMUSERSTATS_INTERFACE_VERSION_011, SteamUserStats_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_001, SteamUtils_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_002, SteamUtils_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_006, SteamUtils_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_007, SteamUtils_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_008, SteamUtils_set_version },
        // { STEAMUTILS_INTERFACE_VERSION_009, SteamUtils_set_version },
        // { STEAMVIDEO_INTERFACE_VERSION_001, SteamVideo_set_version },
        // { STEAMVIDEO_INTERFACE_VERSION_002, SteamVideo_set_version },
        { NULL, NULL }
    };

    GlobalNative gn;

    /* Load SteamAPI library */
    void* h;
    dlerror();
    h = dlopen("libsteam_api.so", RTLD_LAZY);
    if (!h) h = dlopen("libsteam_api64.so", RTLD_LAZY);

    if (!h) {
        char* error = dlerror();
        debuglogstdio(LCF_STEAM | LCF_WARNING, "Could not load Steam library: %s", error?error:"");
        return false;
    }

    /* Find SteamAPI library path */
    struct link_map *l;
    int ret = dlinfo(h, RTLD_DI_LINKMAP, &l);

    if (ret == -1) {
        debuglog(LCF_STEAM | LCF_WARNING, "Could not find Steam library path");
        return false;
    }

    char* steam_path = l->l_name;

    /* Find Steam interface version from the library.
     * Taken from https://git.bitmycode.com/Booti386/DummySteamAPI */

    FILE *fp = fopen(steam_path, "rb");
    if (!fp) {
        debuglog(LCF_STEAM | LCF_WARNING, "Could not open Steam library path");
        return false;
    }

    ssize_t old = ftell(fp);
    fseek(fp, 0, SEEK_END);
    ssize_t size = ftell(fp);
    fseek(fp, old, SEEK_SET);

    if (size <= 0)
    {
        debuglog(LCF_STEAM | LCF_WARNING, "Steam library is empty");
        fclose(fp);
        return false;
    }

    char* data = static_cast<char*>(malloc(size));
    if (!data) {
        debuglog(LCF_STEAM | LCF_WARNING, "No memory");
        fclose(fp);
        return false;
    }

    ret = fread(data, size, 1, fp);
    if (ret != 1) {
        debuglog(LCF_STEAM | LCF_WARNING, "Failed to read from ", steam_path);
        free(data);
        fclose(fp);
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        char *d = &data[i];
        int j = 0;

        while (ifaces[j].name)
        {
            size_t name_len = strlen(ifaces[j].name) + 1;
            size_t remaining_len = size - i;

            if (remaining_len < name_len)
            {
                j++;
                continue;
            }

            if (strncmp(ifaces[j].name, d, name_len) == 0)
            {
                if (ifaces[j].iface_set_default_version)
                    ifaces[j].iface_set_default_version(ifaces[j].name);

                i += name_len - 1;
                break;
            }
            j++;
        }
    }

    return true;
}


bool SteamAPI_Init()
{
    debuglog(LCF_STEAM, __func__, " call.");
    if (shared_config.virtual_steam)
        SteamGetInterfaceVersion();
    return shared_config.virtual_steam;
}

bool SteamAPI_InitSafe()
{
    debuglog(LCF_STEAM, __func__, " call.");
    if (shared_config.virtual_steam)
        SteamGetInterfaceVersion();
    return shared_config.virtual_steam;
}

void SteamAPI_Shutdown()
{
    debuglog(LCF_STEAM, __func__, " call.");
    return;
}

bool SteamAPI_IsSteamRunning()
{
    debuglog(LCF_STEAM, __func__, " call.");
    return shared_config.virtual_steam;
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    debuglog(LCF_STEAM, __func__, " call.");
    return false;
}

void SteamAPI_RunCallbacks()
{
    debuglog(LCF_STEAM, __func__, " call.");
    CCallbackManager::Run();
}

void SteamAPI_RegisterCallback( CCallbackBase *pCallback, enum steam_callback_type iCallback )
{
    debuglogstdio(LCF_STEAM, "%s called with type %d", __func__, iCallback);
    CCallbackManager::RegisterCallback(pCallback, iCallback);
}

void SteamAPI_UnregisterCallback( CCallbackBase *pCallback )
{
    debuglog(LCF_STEAM, __func__, " call.");
    CCallbackManager::UnregisterCallback(pCallback);
}

void SteamAPI_RegisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    debuglog(LCF_STEAM, __func__, " call.");
    CCallbackManager::RegisterApiCallResult(pCallback, hAPICall);
}

void SteamAPI_UnregisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    debuglog(LCF_STEAM, __func__, " call.");
    CCallbackManager::UnregisterApiCallResult(pCallback, hAPICall);
}

ISteamClient *SteamClient()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamClient steamclient;
    return &steamclient;
}

ISteamController *SteamController()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamController steamcontroller;
    return &steamcontroller;
}

ISteamUserStats *SteamUserStats()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUserStats steamuserstats;
    return &steamuserstats;
}

ISteamUser *SteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUser steamuser;
    return &steamuser;
}

ISteamUtils *SteamUtils()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUtils steamutils;
    return &steamutils;
}

ISteamApps *SteamApps()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamApps steamapps;
    return &steamapps;
}

ISteamFriends *SteamFriends()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamFriends steamfriends;
    return &steamfriends;
}

ISteamScreenshots *SteamScreenshots()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamScreenshots steamscreenshots;
    return &steamscreenshots;
}

ISteamUGC *SteamUGC()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamUGC steamugc;
    return &steamugc;
}

ISteamMatchmaking *SteamMatchmaking()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamMatchmaking steammatchmaking;
    return &steammatchmaking;
}

ISteamNetworking *SteamNetworking()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamNetworking steamnetworking;
    return &steamnetworking;
}

ISteamMatchmakingServers *SteamMatchmakingServers()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamMatchmakingServers steammatchmakingservers;
    return &steammatchmakingservers;
}

ISteamHTTP *SteamHTTP()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    static ISteamHTTP steamhttp;
    return &steamhttp;
}

ISteamAppList *SteamAppList()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamMusic *SteamMusic()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamMusicRemote *SteamMusicRemote()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamHTMLSurface *SteamHTMLSurface()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamInventory *SteamInventory()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamVideo *SteamVideo()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

ISteamParentalSettings *SteamParentalSettings()
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    if (!shared_config.virtual_steam)
        return nullptr;

    return nullptr;
}

}
