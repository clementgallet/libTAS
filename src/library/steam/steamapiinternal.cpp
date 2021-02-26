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

#include "steamapiinternal.h"
#include "../logging.h"
#include "steamapi.h"
// #include <signal.h>
#include <dlfcn.h>

namespace libtas {

HSteamUser SteamAPI_GetHSteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);
    if (auto *user = SteamUser())
        return user->GetHSteamUser();
    return 0;
}

HSteamPipe SteamAPI_GetHSteamPipe()
{
    DEBUGLOGCALL(LCF_STEAM);
    return shared_config.virtual_steam;
}

void * SteamInternal_ContextInit( void *pContextInitData )
{
    DEBUGLOGCALL(LCF_STEAM);
    static CSteamAPIContext context;
    GlobalNoLog gnl;
    context.m_pSteamClient = SteamClient();
    context.m_pSteamUser = SteamUser();
    context.m_pSteamUserStats = SteamUserStats();
    context.m_pSteamUtils = SteamUtils();
    context.m_pSteamRemoteStorage = SteamRemoteStorage();
    context.m_pSteamApps = SteamApps();
    context.m_pSteamFriends = SteamFriends();
    context.m_pSteamScreenshots = SteamScreenshots();
    context.m_pSteamUGC = SteamUGC();
    context.m_pSteamMatchmaking = SteamMatchmaking();
    context.m_pSteamMatchmakingServers = SteamMatchmakingServers();
    context.m_pSteamHTTP = SteamHTTP();
    context.m_pSteamNetworking = SteamNetworking();
    return &context;
}

void * SteamInternal_CreateInterface( const char *ver )
{
    debuglogstdio(LCF_STEAM, "%s called with %s", __func__, ver);
    if (!shared_config.virtual_steam)
        return nullptr;

    /* The expected return from this function is a pointer to a C++ class with
     * specific virtual functions.  The format of our argument is the name
     * of the corresponding C function that has already been hooked to return
     * the correct value, followed by some numbers that are probably used for
     * version checking.  As a quick hack, just lookup the symbol and call it.
     */
    std::string symbol = ver;
    /* Strip numbers at the end */
    auto end = symbol.find_last_not_of("0123456789");
    if (end != std::string::npos)
        symbol.resize(end + 1);
    void *(*func)() = reinterpret_cast<void *(*)()>(dlsym(RTLD_DEFAULT, symbol.c_str()));
    if (func)
        return func();
    return nullptr;
}

bool _ZN16CSteamAPIContext4InitEv(CSteamAPIContext* context)
{
    DEBUGLOGCALL(LCF_STEAM);
    GlobalNoLog gnl;
    context->m_pSteamClient = SteamClient();
    context->m_pSteamUser = SteamUser();
    context->m_pSteamUserStats = SteamUserStats();
    context->m_pSteamUtils = SteamUtils();
    context->m_pSteamRemoteStorage = SteamRemoteStorage();
    context->m_pSteamApps = SteamApps();
    context->m_pSteamFriends = SteamFriends();
    context->m_pSteamScreenshots = SteamScreenshots();
    context->m_pSteamUGC = SteamUGC();
    context->m_pSteamMatchmaking = SteamMatchmaking();
    context->m_pSteamMatchmakingServers = SteamMatchmakingServers();
    context->m_pSteamHTTP = SteamHTTP();
    return true;
}

}
