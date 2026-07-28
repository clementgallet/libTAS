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

#include "steamapi.h"
#include "CCallbackManager.h"
#include "isteamremotestorage/isteamremotestorage001.h"
#include "isteamremotestorage/isteamremotestorage002.h"
#include "isteamremotestorage/isteamremotestorage003.h"
#include "isteamremotestorage/isteamremotestorage005.h"
#include "isteamremotestorage/isteamremotestorage007.h"
#include "isteamremotestorage/isteamremotestorage012.h"
#include "isteamremotestorage/isteamremotestorage013.h"
#include "isteamremotestorage/isteamremotestorage014.h"
#include "isteamclient/isteamclient006.h"
#include "isteamclient/isteamclient012.h"
#include "isteamclient/isteamclient014.h"
#include "isteamclient/isteamclient016.h"
#include "isteamclient/isteamclient017.h"
#include "isteamclient/isteamclient020.h"
#include "isteamclient/isteamclient021.h"
#include "isteamuser/isteamuser021.h"
#include "isteamuser/isteamuser023.h"
#include "isteamuserstats/isteamuserstats005.h"
#include "isteamuserstats/isteamuserstats006.h"
#include "isteamuserstats/isteamuserstats007.h"
#include "isteamuserstats/isteamuserstats008.h"
#include "isteamuserstats/isteamuserstats010.h"
#include "isteamuserstats/isteamuserstats011.h"
#include "isteamuserstats/isteamuserstats012.h"
#include "isteamuserstats/isteamuserstats013.h"

#include "logging.h"
#include "hook.h"
#include "global.h"
#include "GlobalState.h"

#include <signal.h>
#ifdef __unix__
#include <link.h>
#endif
#include <dlfcn.h>

namespace libtas {

const steam_interface* SteamGetAllInterfaces() {
    static const steam_interface ifaces[] = {
        // { STEAMAPPLIST_INTERFACE_VERSION_001, SteamAppList_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_001, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_003, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_005, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_006, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_007, SteamApps_set_version },
        // { STEAMAPPS_INTERFACE_VERSION_008, SteamApps_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_006, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_007, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_008, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_009, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_010, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_011, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_012, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_013, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_014, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_015, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_016, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_017, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_018, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_019, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_020, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
        { STEAMCLIENT_INTERFACE_VERSION_021, SteamClient_set_version, reinterpret_cast<void *(*)(const char*)>(SteamClient_generic) },
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
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_001, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_002, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_003, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_004, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_005, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_006, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_007, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_008, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_009, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_010, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_011, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_012, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_013, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
        { STEAMREMOTESTORAGE_INTERFACE_VERSION_014, SteamRemoteStorage_set_version, reinterpret_cast<void *(*)(const char*)>(SteamRemoteStorage_generic) },
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
        { STEAMUSER_INTERFACE_VERSION_016, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_017, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_018, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_019, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_020, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_021, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSER_INTERFACE_VERSION_023, SteamUser_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUser_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_003, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_004, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_005, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_006, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_007, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_008, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_009, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_010, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_011, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_012, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
        { STEAMUSERSTATS_INTERFACE_VERSION_013, SteamUserStats_set_version, reinterpret_cast<void *(*)(const char*)>(SteamUserStats_generic) },
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

    return ifaces;
}

static bool SteamGetInterfaceVersion()
{
    GlobalNative gn;

    /* Load SteamAPI library */
    void* h;
    dlerror();
    h = dlopen("libsteam_api.so", RTLD_LAZY);
    if (!h) h = dlopen("libsteam_api64.so", RTLD_LAZY);

    if (!h) {
        char* error = dlerror();
        LOG(LL_WARN, LCF_STEAM, "Could not load Steam library: %s", error?error:"");
        return false;
    }

/* Find SteamAPI library path */
#ifdef __unix__
    struct link_map *l;
    int ret = dlinfo(h, RTLD_DI_LINKMAP, &l);

    if (ret == -1) {
        LOG(LL_WARN, LCF_STEAM, "Could not find Steam library path");
        dlclose(h);
        return false;
    }

    char* steam_path = l->l_name;

#elif defined(__APPLE__) && defined(__MACH__)
    void* f = dlsym(h, "SteamAPI_Init");
    if (!f) {
        LOG(LL_WARN, LCF_STEAM, "Could not find a symbol inside Steam library");
        dlclose(h);
        return false;        
    }
    
    Dl_info dli;
    int ret = dladdr(f, &dli);

    if (ret == 0) {
        LOG(LL_WARN, LCF_STEAM, "Could not find address of Steam symbol");
        dlclose(h);
        return false;
    }
    
    const char* steam_path = dli.dli_fname;
#endif

    dlclose(h);

    /* Find Steam interface version from the library.
     * Taken from https://git.bitmycode.com/Booti386/DummySteamAPI */

    FILE *fp = fopen(steam_path, "rb");
    if (!fp) {
        LOG(LL_WARN, LCF_STEAM, "Could not open Steam library path");
        return false;
    }

    ssize_t old = ftell(fp);
    fseek(fp, 0, SEEK_END);
    ssize_t size = ftell(fp);
    fseek(fp, old, SEEK_SET);

    if (size <= 0)
    {
        LOG(LL_WARN, LCF_STEAM, "Steam library is empty");
        fclose(fp);
        return false;
    }

    char* data = static_cast<char*>(malloc(size));
    if (!data) {
        LOG(LL_WARN, LCF_STEAM, "No memory");
        fclose(fp);
        return false;
    }

    ret = fread(data, size, 1, fp);
    if (ret != 1) {
        LOG(LL_WARN, LCF_STEAM, "Failed to read from %s", steam_path);
        free(data);
        fclose(fp);
        return false;
    }

    const steam_interface* ifaces = SteamGetAllInterfaces();

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

    fclose(fp);
    return true;
}

bool SteamAPI_Init()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam) {
        SteamGetInterfaceVersion();
        return true;
    }
    RETURN_NATIVE(SteamAPI_Init, (), "steam_api");
}

bool SteamAPI_InitSafe()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam) {
        SteamGetInterfaceVersion();
        return true;
    }
    RETURN_NATIVE(SteamAPI_InitSafe, (), "steam_api");
}

void SteamAPI_Shutdown()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
    
    RETURN_NATIVE(SteamAPI_Shutdown, (), "steam_api");
}

bool SteamAPI_IsSteamRunning()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return true;
    
    RETURN_NATIVE(SteamAPI_IsSteamRunning, (), "steam_api");
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return false;

    RETURN_NATIVE(SteamAPI_RestartAppIfNecessary, (unOwnAppID), "steam_api");
}

void SteamAPI_RunCallbacks()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::Run();

    RETURN_NATIVE(SteamAPI_RunCallbacks, (), "steam_api");
}

void SteamAPI_RegisterCallback( CCallbackBase *pCallback, enum steam_callback_type iCallback )
{
    LOGTRACE(LCF_STEAM, "%s called with type %d", __func__, iCallback);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::RegisterCallback(pCallback, iCallback);

    RETURN_NATIVE(SteamAPI_RegisterCallback, (pCallback, iCallback), "steam_api");
}

void SteamAPI_UnregisterCallback( CCallbackBase *pCallback )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::UnregisterCallback(pCallback);

    RETURN_NATIVE(SteamAPI_UnregisterCallback, (pCallback), "steam_api");
}

void SteamAPI_RegisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::RegisterApiCallResult(pCallback, hAPICall);

    RETURN_NATIVE(SteamAPI_RegisterCallResult, (pCallback, hAPICall), "steam_api");
}

void SteamAPI_UnregisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::UnregisterApiCallResult(pCallback, hAPICall);
        
    RETURN_NATIVE(SteamAPI_UnregisterCallResult, (pCallback, hAPICall), "steam_api");
}

void SteamAPI_ManualDispatch_Init()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    RETURN_NATIVE(SteamAPI_ManualDispatch_Init, (), "steam_api");
}

void SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    RETURN_NATIVE(SteamAPI_ManualDispatch_RunFrame, (hSteamPipe), "steam_api");
}

bool SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return false;
        
    RETURN_NATIVE(SteamAPI_ManualDispatch_GetNextCallback, (hSteamPipe, pCallbackMsg), "steam_api");
}

void SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    RETURN_NATIVE(SteamAPI_ManualDispatch_FreeLastCallback, (hSteamPipe), "steam_api");
}

bool SteamAPI_ManualDispatch_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (pbFailed)
        *pbFailed = false;

    if (Global::shared_config.virtual_steam) {
        if (iCallbackExpected < 0 || iCallbackExpected >= STEAM_CALLBACK_TYPE_MAX)
            return false;

        return CCallbackManager::ApiCallResultGetOutput(hSteamAPICall, pCallback, cubCallback, static_cast<steam_callback_type>(iCallbackExpected), pbFailed);
    }
    
    RETURN_NATIVE(SteamAPI_ManualDispatch_GetAPICallResult, (hSteamPipe, hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed), "steam_api");
}

ISteamController *SteamController()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamController, (), "steam_api");
    }

    static ISteamController steamcontroller;
    return &steamcontroller;
}

ISteamUtils *SteamUtils()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamUtils, (), "steam_api");
    }

    static ISteamUtils steamutils;
    return &steamutils;
}

ISteamApps *SteamApps()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamApps, (), "steam_api");
    }

    static ISteamApps steamapps;
    return &steamapps;
}

ISteamFriends *SteamFriends()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamFriends, (), "steam_api");
    }

    static ISteamFriends steamfriends;
    return &steamfriends;
}

ISteamScreenshots *SteamScreenshots()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamScreenshots, (), "steam_api");
    }

    static ISteamScreenshots steamscreenshots;
    return &steamscreenshots;
}

ISteamUGC *SteamUGC()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamUGC, (), "steam_api");
    }

    static ISteamUGC steamugc;
    return &steamugc;
}

ISteamMatchmaking *SteamMatchmaking()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamMatchmaking, (), "steam_api");
    }

    static ISteamMatchmaking steammatchmaking;
    return &steammatchmaking;
}

ISteamNetworking *SteamNetworking()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamNetworking, (), "steam_api");
    }

    static ISteamNetworking steamnetworking;
    return &steamnetworking;
}

ISteamMatchmakingServers *SteamMatchmakingServers()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamMatchmakingServers, (), "steam_api");
    }

    static ISteamMatchmakingServers steammatchmakingservers;
    return &steammatchmakingservers;
}

ISteamHTTP *SteamHTTP()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamHTTP, (), "steam_api");
    }

    static ISteamHTTP steamhttp;
    return &steamhttp;
}

ISteamAppList *SteamAppList()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamAppList, (), "steam_api");
    }

    return nullptr;
}

ISteamMusic *SteamMusic()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamMusic, (), "steam_api");
    }

    return nullptr;
}

ISteamMusicRemote *SteamMusicRemote()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamMusicRemote, (), "steam_api");
    }

    return nullptr;
}

ISteamHTMLSurface *SteamHTMLSurface()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamHTMLSurface, (), "steam_api");
    }

    return nullptr;
}

ISteamInventory *SteamInventory()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamInventory, (), "steam_api");
    }

    return nullptr;
}

ISteamVideo *SteamVideo()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamVideo, (), "steam_api");
    }

    return nullptr;
}

ISteamParentalSettings *SteamParentalSettings()
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamParentalSettings, (), "steam_api");
    }

    return nullptr;
}

ISteamNetworkingUtils *SteamNetworkingUtils()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamNetworkingUtils, (), "steam_api");
    }

    static ISteamNetworkingUtils steamnetworkingutils;
    return &steamnetworkingutils;
}

ISteamNetworkingSockets *SteamNetworkingSockets()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamNetworkingSockets, (), "steam_api");
    }

    static ISteamNetworkingSockets steamnetworkingsockets;
    return &steamnetworkingsockets;
}

ISteamNetworkingMessages *SteamNetworkingMessages()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamNetworkingMessages, (), "steam_api");
    }

    static ISteamNetworkingMessages steamnetworkingmessages;
    return &steamnetworkingmessages;
}

ISteamInput *SteamInput()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        RETURN_NATIVE(SteamInput, (), "steam_api");
    }

    static ISteamInput steaminput;
    return &steaminput;
}

}
