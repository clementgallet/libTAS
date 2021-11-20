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

#include "isteamclient.h"
#include "isteamclient_priv.h"
#include "isteamclient006.h"
#include "isteamclient012.h"
#include "isteamclient014.h"
#include "isteamclient016.h"
#include "isteamclient017.h"
#include "isteamclient020.h"
#include "../isteamcontroller.h"
#include "../steamapi.h"
#include "../steamgameserver.h"
#include "../isteamgamecoordinator.h"
#include "../../logging.h"
#include "../../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(SteamClient)

static const char *steamclient_version = NULL;

struct ISteamClient *SteamClient_generic(const char *version)
{
	static const struct
	{
		const char *name;
		struct ISteamClient *(*iface_getter)(void);
	} ifaces[] = {
        { STEAMCLIENT_INTERFACE_VERSION_006, SteamClient006 },
		{ STEAMCLIENT_INTERFACE_VERSION_012, SteamClient012 },
		{ STEAMCLIENT_INTERFACE_VERSION_014, SteamClient014 },
		{ STEAMCLIENT_INTERFACE_VERSION_016, SteamClient016 },
		{ STEAMCLIENT_INTERFACE_VERSION_017, SteamClient017 },
        { STEAMCLIENT_INTERFACE_VERSION_020, SteamClient020 },
		{ NULL, NULL }
	};
	int i;

    debuglogstdio(LCF_STEAM, "%s called with version %s", __func__, version);

	i = 0;
	while (ifaces[i].name)
	{
		if (strcmp(ifaces[i].name, version) == 0)
		{
			if (ifaces[i].iface_getter)
				return ifaces[i].iface_getter();

			break;
		}
		i++;
	}

    debuglogstdio(LCF_STEAM | LCF_WARNING, "Unable to find ISteamClient version %s", version);

	return nullptr;
}

void SteamClient_set_version(const char *version)
{
    debuglogstdio(LCF_STEAM, "%s called with version %s", __func__, version);

	// if (!steamclient_version)
		steamclient_version = version;
}

struct ISteamClient *SteamClient(void)
{
    DEBUGLOGCALL(LCF_STEAM);

    if (!shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamClient, "steam_api");
        return orig::SteamClient();
    }

	static struct ISteamClient *cached_iface = nullptr;

	if (!steamclient_version)
	{
		steamclient_version = STEAMCLIENT_INTERFACE_VERSION_017;
        debuglogstdio(LCF_STEAM | LCF_WARNING, "ISteamClient: No version specified, defaulting to %s", steamclient_version);
	}

	if (!cached_iface)
		cached_iface = SteamClient_generic(steamclient_version);

	return cached_iface;
}

HSteamPipe ISteamClient_CreateSteamPipe(void *iface)
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamClient_BReleaseSteamPipe( void *iface, HSteamPipe hSteamPipe )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

HSteamUser ISteamClient_ConnectToGlobalUser( void *iface, HSteamPipe hSteamPipe )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamUser ISteamClient_CreateLocalUser( void *iface, HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamClient_ReleaseUser( void *iface, HSteamPipe hSteamPipe, HSteamUser hUser )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamUser *ISteamClient_GetISteamUser( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUser();
}

ISteamGameServer *ISteamClient_GetISteamGameServer( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamGameServer();
}

void ISteamClient_SetLocalIPBinding( void *iface, uint32_t unIP, uint16_t usPort )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamFriends *ISteamClient_GetISteamFriends( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamFriends();
}

ISteamUtils *ISteamClient_GetISteamUtils( void *iface, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUtils();
}

ISteamMatchmaking *ISteamClient_GetISteamMatchmaking( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamMatchmaking();
}

ISteamMatchmakingServers *ISteamClient_GetISteamMatchmakingServers( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamMatchmakingServers();
}

void *ISteamClient_GetISteamGenericInterface( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    if (strcmp(pchVersion, "SteamGameCoordinator001") == 0) {
        static ISteamGameCoordinator steamgamecoordinator;
        return &steamgamecoordinator;
    }
    debuglogstdio(LCF_STEAM | LCF_ERROR, "Invalid interface %s", pchVersion);
    return nullptr;
}

ISteamUserStats *ISteamClient_GetISteamUserStats( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUserStats();
}

ISteamGameServerStats *ISteamClient_GetISteamGameServerStats( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamApps *ISteamClient_GetISteamApps( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamApps();
}

ISteamNetworking *ISteamClient_GetISteamNetworking( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamNetworking();
}

ISteamRemoteStorage *ISteamClient_GetISteamRemoteStorage( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamRemoteStorage();
}

ISteamScreenshots *ISteamClient_GetISteamScreenshots( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamScreenshots();
}

ISteamGameSearch *ISteamClient_GetISteamGameSearch(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void ISteamClient_RunFrame(void *iface)
{
    DEBUGLOGCALL(LCF_STEAM);
}

unsigned int ISteamClient_GetIPCCallCount(void *iface)
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamClient_SetWarningMessageHook( void *iface, void *pFunction )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamClient_BShutdownIfAllPipesClosed(void *iface)
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

ISteamHTTP *ISteamClient_GetISteamHTTP( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamHTTP();
}

void *ISteamClient_GetISteamUnifiedMessages( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamController *ISteamClient_GetISteamController( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamController();
}

ISteamUGC *ISteamClient_GetISteamUGC( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return SteamUGC();
}

ISteamAppList *ISteamClient_GetISteamAppList( void *iface, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamMusic *ISteamClient_GetISteamMusic( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamMusicRemote *ISteamClient_GetISteamMusicRemote(void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamHTMLSurface *ISteamClient_GetISteamHTMLSurface(void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

void ISteamClient_DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void *iface, void (*)() )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamClient_DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void *iface, void (*)() )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamClient_Set_SteamAPI_CCheckCallbackRegisteredInProcess( void *iface, SteamAPI_CheckCallbackRegistered_t func )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ISteamInventory *ISteamClient_GetISteamInventory( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamVideo *ISteamClient_GetISteamVideo( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamParentalSettings *ISteamClient_GetISteamParentalSettings( void *iface, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamInput *ISteamClient_GetISteamInput(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamParties *ISteamClient_GetISteamParties(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

ISteamRemotePlay *ISteamClient_GetISteamRemotePlay(void *iface, HSteamUser steam_user, HSteamPipe steam_pipe, const char *version)
{
    DEBUGLOGCALL(LCF_STEAM);
    return reinterpret_cast<void*>(1); // Return a value that evaluates to `true`
}

}
