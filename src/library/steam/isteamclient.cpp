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

#include "isteamclient.h"
#include "isteamcontroller.h"
#include "steamapi.h"
#include "../logging.h"

namespace libtas {

HSteamPipe ISteamClient::CreateSteamPipe()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamClient::BReleaseSteamPipe( HSteamPipe hSteamPipe )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

HSteamUser ISteamClient::ConnectToGlobalUser( HSteamPipe hSteamPipe )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamUser ISteamClient::CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamClient::ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamUser *ISteamClient::GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUser();
}

ISteamGameServer *ISteamClient::GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void ISteamClient::SetLocalIPBinding( uint32_t unIP, uint16_t usPort )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamFriends *ISteamClient::GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamFriends();
}

ISteamUtils *ISteamClient::GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUtils();
}

ISteamMatchmaking *ISteamClient::GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamMatchmakingServers *ISteamClient::GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void *ISteamClient::GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamUserStats *ISteamClient::GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUserStats();
}

ISteamGameServerStats *ISteamClient::GetISteamGameServerStats( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamApps *ISteamClient::GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamApps();
}

ISteamNetworking *ISteamClient::GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamRemoteStorage *ISteamClient::GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamRemoteStorage();
}

ISteamScreenshots *ISteamClient::GetISteamScreenshots( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamScreenshots();
}

void ISteamClient::RunFrame()
{
    DEBUGLOGCALL(LCF_STEAM);
}

unsigned int ISteamClient::GetIPCCallCount()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamClient::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamClient::BShutdownIfAllPipesClosed()
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

ISteamHTTP *ISteamClient::GetISteamHTTP( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void *ISteamClient::DEPRECATED_GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamController *ISteamClient::GetISteamController( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamController();
}

ISteamUGC *ISteamClient::GetISteamUGC( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUGC();
}

ISteamAppList *ISteamClient::GetISteamAppList( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamMusic *ISteamClient::GetISteamMusic( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamMusicRemote *ISteamClient::GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamHTMLSurface *ISteamClient::GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void ISteamClient::DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamClient::DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamClient::Set_SteamAPI_CCheckCallbackRegisteredInProcess( SteamAPI_CheckCallbackRegistered_t func )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamInventory *ISteamClient::GetISteamInventory( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamVideo *ISteamClient::GetISteamVideo( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamParentalSettings *ISteamClient::GetISteamParentalSettings( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

}
