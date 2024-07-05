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

#include "steamapi.h"
#include "CCallbackManager.h"
#include "isteamremotestorage/isteamremotestorage001.h"
#include "isteamremotestorage/isteamremotestorage012.h"
#include "isteamremotestorage/isteamremotestorage013.h"
#include "isteamremotestorage/isteamremotestorage014.h"
#include "isteamclient/isteamclient006.h"
#include "isteamclient/isteamclient012.h"
#include "isteamclient/isteamclient014.h"
#include "isteamclient/isteamclient016.h"
#include "isteamclient/isteamclient017.h"
#include "isteamclient/isteamclient020.h"

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
        { STEAMCLIENT_INTERFACE_VERSION_006, SteamClient_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_012, SteamClient_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_014, SteamClient_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_016, SteamClient_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_017, SteamClient_set_version },
        { STEAMCLIENT_INTERFACE_VERSION_020, SteamClient_set_version },
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
        LOG(LL_WARN, LCF_STEAM, "Could not load Steam library: %s", error?error:"");
        return false;
    }

/* Find SteamAPI library path */
#ifdef __unix__
    struct link_map *l;
    int ret = dlinfo(h, RTLD_DI_LINKMAP, &l);

    if (ret == -1) {
        LOG(LL_WARN, LCF_STEAM, "Could not find Steam library path");
        return false;
    }

    char* steam_path = l->l_name;

#elif defined(__APPLE__) && defined(__MACH__)
    void* f = dlsym(h, "SteamAPI_Init");
    if (!f) {
        LOG(LL_WARN, LCF_STEAM, "Could not find a symbol inside Steam library");
        return false;        
    }
    
    Dl_info dli;
    int ret = dladdr(f, &dli);

    if (ret == 0) {
        LOG(LL_WARN, LCF_STEAM, "Could not find address of Steam symbol");
        return false;
    }
    
    const char* steam_path = dli.dli_fname;
#endif

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

DEFINE_ORIG_POINTER(SteamAPI_Init)
DEFINE_ORIG_POINTER(SteamAPI_InitSafe)
DEFINE_ORIG_POINTER(SteamAPI_Shutdown)
DEFINE_ORIG_POINTER(SteamAPI_IsSteamRunning)
DEFINE_ORIG_POINTER(SteamAPI_RestartAppIfNecessary)
DEFINE_ORIG_POINTER(SteamAPI_RunCallbacks)
DEFINE_ORIG_POINTER(SteamAPI_RegisterCallback)
DEFINE_ORIG_POINTER(SteamAPI_UnregisterCallback)
DEFINE_ORIG_POINTER(SteamAPI_RegisterCallResult)
DEFINE_ORIG_POINTER(SteamAPI_UnregisterCallResult)

DEFINE_ORIG_POINTER(SteamAPI_ManualDispatch_Init)
DEFINE_ORIG_POINTER(SteamAPI_ManualDispatch_RunFrame)
DEFINE_ORIG_POINTER(SteamAPI_ManualDispatch_GetNextCallback)
DEFINE_ORIG_POINTER(SteamAPI_ManualDispatch_FreeLastCallback)
DEFINE_ORIG_POINTER(SteamAPI_ManualDispatch_GetAPICallResult)

DEFINE_ORIG_POINTER(SteamController)
DEFINE_ORIG_POINTER(SteamUserStats)
DEFINE_ORIG_POINTER(SteamUser)
DEFINE_ORIG_POINTER(SteamUtils)
DEFINE_ORIG_POINTER(SteamApps)
DEFINE_ORIG_POINTER(SteamFriends)
DEFINE_ORIG_POINTER(SteamScreenshots)
DEFINE_ORIG_POINTER(SteamUGC)
DEFINE_ORIG_POINTER(SteamMatchmaking)
DEFINE_ORIG_POINTER(SteamNetworking)
DEFINE_ORIG_POINTER(SteamMatchmakingServers)
DEFINE_ORIG_POINTER(SteamHTTP)
DEFINE_ORIG_POINTER(SteamAppList)
DEFINE_ORIG_POINTER(SteamMusic)
DEFINE_ORIG_POINTER(SteamMusicRemote)
DEFINE_ORIG_POINTER(SteamHTMLSurface)
DEFINE_ORIG_POINTER(SteamInventory)
DEFINE_ORIG_POINTER(SteamVideo)
DEFINE_ORIG_POINTER(SteamParentalSettings)
DEFINE_ORIG_POINTER(SteamNetworkingUtils)
DEFINE_ORIG_POINTER(SteamNetworkingSockets)
DEFINE_ORIG_POINTER(SteamNetworkingMessages)
DEFINE_ORIG_POINTER(SteamInput)

bool SteamAPI_Init()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam) {
        SteamGetInterfaceVersion();
        return true;
    }
    LINK_NAMESPACE(SteamAPI_Init, "steam_api");
    return orig::SteamAPI_Init();
}

bool SteamAPI_InitSafe()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam) {
        SteamGetInterfaceVersion();
        return true;
    }
    LINK_NAMESPACE(SteamAPI_InitSafe, "steam_api");
    return orig::SteamAPI_InitSafe();
}

void SteamAPI_Shutdown()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
    
    LINK_NAMESPACE(SteamAPI_Shutdown, "steam_api");
    return orig::SteamAPI_Shutdown();
}

bool SteamAPI_IsSteamRunning()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return true;
    
    LINK_NAMESPACE(SteamAPI_IsSteamRunning, "steam_api");
    return orig::SteamAPI_IsSteamRunning();
}

bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return false;

    LINK_NAMESPACE(SteamAPI_RestartAppIfNecessary, "steam_api");
    return orig::SteamAPI_RestartAppIfNecessary(unOwnAppID);
}

void SteamAPI_RunCallbacks()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::Run();

    LINK_NAMESPACE(SteamAPI_RunCallbacks, "steam_api");
    return orig::SteamAPI_RunCallbacks();
}

void SteamAPI_RegisterCallback( CCallbackBase *pCallback, enum steam_callback_type iCallback )
{
    LOG(LL_TRACE, LCF_STEAM, "%s called with type %d", __func__, iCallback);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::RegisterCallback(pCallback, iCallback);

    LINK_NAMESPACE(SteamAPI_RegisterCallback, "steam_api");
    return orig::SteamAPI_RegisterCallback(pCallback, iCallback);
}

void SteamAPI_UnregisterCallback( CCallbackBase *pCallback )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::UnregisterCallback(pCallback);

    LINK_NAMESPACE(SteamAPI_UnregisterCallback, "steam_api");
    return orig::SteamAPI_UnregisterCallback(pCallback);
}

void SteamAPI_RegisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::RegisterApiCallResult(pCallback, hAPICall);

    LINK_NAMESPACE(SteamAPI_RegisterCallResult, "steam_api");
    return orig::SteamAPI_RegisterCallResult(pCallback, hAPICall);
}

void SteamAPI_UnregisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return CCallbackManager::UnregisterApiCallResult(pCallback, hAPICall);
        
    LINK_NAMESPACE(SteamAPI_UnregisterCallResult, "steam_api");
    return orig::SteamAPI_UnregisterCallResult(pCallback, hAPICall);
}

void SteamAPI_ManualDispatch_Init()
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    LINK_NAMESPACE(SteamAPI_ManualDispatch_Init, "steam_api");
    return orig::SteamAPI_ManualDispatch_Init();
}

void SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    LINK_NAMESPACE(SteamAPI_ManualDispatch_RunFrame, "steam_api");
    return orig::SteamAPI_ManualDispatch_RunFrame(hSteamPipe);
}

bool SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return false;
        
    LINK_NAMESPACE(SteamAPI_ManualDispatch_GetNextCallback, "steam_api");
    return orig::SteamAPI_ManualDispatch_GetNextCallback(hSteamPipe, pCallbackMsg);
}

void SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return;
        
    LINK_NAMESPACE(SteamAPI_ManualDispatch_FreeLastCallback, "steam_api");
    return orig::SteamAPI_ManualDispatch_FreeLastCallback(hSteamPipe);
}

bool SteamAPI_ManualDispatch_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed )
{
    LOGTRACE(LCF_STEAM);
    if (Global::shared_config.virtual_steam)
        return false;
        
    LINK_NAMESPACE(SteamAPI_ManualDispatch_GetAPICallResult, "steam_api");
    return orig::SteamAPI_ManualDispatch_GetAPICallResult(hSteamPipe, hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
}

ISteamController *SteamController()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamController, "steam_api");
        return orig::SteamController();
    }

    static ISteamController steamcontroller;
    return &steamcontroller;
}

ISteamUserStats *SteamUserStats()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUserStats, "steam_api");
        return orig::SteamUserStats();
    }

    static ISteamUserStats steamuserstats;
    return &steamuserstats;
}

ISteamUser *SteamUser()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUser, "steam_api");
        return orig::SteamUser();
    }

    static ISteamUser steamuser;
    return &steamuser;
}

ISteamUtils *SteamUtils()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUtils, "steam_api");
        return orig::SteamUtils();
    }

    static ISteamUtils steamutils;
    return &steamutils;
}

ISteamApps *SteamApps()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamApps, "steam_api");
        return orig::SteamApps();
    }

    static ISteamApps steamapps;
    return &steamapps;
}

ISteamFriends *SteamFriends()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamFriends, "steam_api");
        return orig::SteamFriends();
    }

    static ISteamFriends steamfriends;
    return &steamfriends;
}

ISteamScreenshots *SteamScreenshots()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamScreenshots, "steam_api");
        return orig::SteamScreenshots();
    }

    static ISteamScreenshots steamscreenshots;
    return &steamscreenshots;
}

ISteamUGC *SteamUGC()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUGC, "steam_api");
        return orig::SteamUGC();
    }

    static ISteamUGC steamugc;
    return &steamugc;
}

ISteamMatchmaking *SteamMatchmaking()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamMatchmaking, "steam_api");
        return orig::SteamMatchmaking();
    }

    static ISteamMatchmaking steammatchmaking;
    return &steammatchmaking;
}

ISteamNetworking *SteamNetworking()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamNetworking, "steam_api");
        return orig::SteamNetworking();
    }

    static ISteamNetworking steamnetworking;
    return &steamnetworking;
}

ISteamMatchmakingServers *SteamMatchmakingServers()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamMatchmakingServers, "steam_api");
        return orig::SteamMatchmakingServers();
    }

    static ISteamMatchmakingServers steammatchmakingservers;
    return &steammatchmakingservers;
}

ISteamHTTP *SteamHTTP()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamHTTP, "steam_api");
        return orig::SteamHTTP();
    }

    static ISteamHTTP steamhttp;
    return &steamhttp;
}

ISteamAppList *SteamAppList()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamAppList, "steam_api");
        return orig::SteamAppList();
    }

    return nullptr;
}

ISteamMusic *SteamMusic()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamMusic, "steam_api");
        return orig::SteamMusic();
    }

    return nullptr;
}

ISteamMusicRemote *SteamMusicRemote()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamMusicRemote, "steam_api");
        return orig::SteamMusicRemote();
    }

    return nullptr;
}

ISteamHTMLSurface *SteamHTMLSurface()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamHTMLSurface, "steam_api");
        return orig::SteamHTMLSurface();
    }

    return nullptr;
}

ISteamInventory *SteamInventory()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamInventory, "steam_api");
        return orig::SteamInventory();
    }

    return nullptr;
}

ISteamVideo *SteamVideo()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamVideo, "steam_api");
        return orig::SteamVideo();
    }

    return nullptr;
}

ISteamParentalSettings *SteamParentalSettings()
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamParentalSettings, "steam_api");
        return orig::SteamParentalSettings();
    }

    return nullptr;
}

ISteamNetworkingUtils *SteamNetworkingUtils()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamNetworkingUtils, "steam_api");
        return orig::SteamNetworkingUtils();
    }

    static ISteamNetworkingUtils steamnetworkingutils;
    return &steamnetworkingutils;
}

ISteamNetworkingSockets *SteamNetworkingSockets()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamNetworkingSockets, "steam_api");
        return orig::SteamNetworkingSockets();
    }

    static ISteamNetworkingSockets steamnetworkingsockets;
    return &steamnetworkingsockets;
}

ISteamNetworkingMessages *SteamNetworkingMessages()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamNetworkingMessages, "steam_api");
        return orig::SteamNetworkingMessages();
    }

    static ISteamNetworkingMessages steamnetworkingmessages;
    return &steamnetworkingmessages;
}

ISteamInput *SteamInput()
{
    LOGTRACE(LCF_STEAM);
    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamInput, "steam_api");
        return orig::SteamInput();
    }

    static ISteamInput steaminput;
    return &steaminput;
}

}
