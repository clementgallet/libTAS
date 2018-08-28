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

#include "steamapiinternal.h"
#include "../logging.h"
#include "steamapi.h"
// #include <signal.h>

namespace libtas {

HSteamUser SteamAPI_GetHSteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void * SteamInternal_ContextInit( void *pContextInitData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

void * SteamInternal_CreateInterface( const char *ver )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

bool _ZN16CSteamAPIContext4InitEv(CSteamAPIContext* context)
{
    DEBUGLOGCALL(LCF_STEAM);
    context->m_pSteamUser = SteamUser();
    context->m_pSteamUserStats = SteamUserStats();
    context->m_pSteamUtils = SteamUtils();
    context->m_pSteamRemoteStorage = SteamRemoteStorage();
    return true;
}

}
