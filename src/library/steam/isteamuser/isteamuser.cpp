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

#include "isteamuser.h"
#include "isteamuser_priv.h"
#include "isteamuser021.h"
#include "isteamuser023.h"

#include "steam/CCallback.h"
#include "steam/CCallbackManager.h"

#include "logging.h"
#include "hook.h"
#include "Utils.h"
#include "global.h"

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h> 

namespace libtas {

DEFINE_ORIG_POINTER(SteamUser)

char steamuserdir[2048] = "/NOTVALID";
static const char *steamuser_version = NULL;

void SteamSetUserDataFolder(std::string path)
{
    LOGTRACE(LCF_STEAM);
    strncpy(steamuserdir, path.c_str(), sizeof(steamuserdir)-1);
}

struct ISteamUser *SteamUser_generic(const char *version)
{
	static const struct
	{
		const char *name;
		struct ISteamUser *(*iface_getter)(void);
	} ifaces[] = {
		{ STEAMUSER_INTERFACE_VERSION_019, SteamUser021 },
		{ STEAMUSER_INTERFACE_VERSION_020, SteamUser021 },
		{ STEAMUSER_INTERFACE_VERSION_021, SteamUser021 },
        { STEAMUSER_INTERFACE_VERSION_023, SteamUser023 },
		{ NULL, NULL }
	};
	int i;

    LOG(LL_DEBUG, LCF_STEAM, "%s called with version %s", __func__, version);

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

    LOG(LL_WARN, LCF_STEAM, "Unable to find ISteamUser version %s", version);

	return nullptr;
}

void SteamUser_set_version(const char *version)
{
    LOG(LL_DEBUG, LCF_STEAM, "%s called with version %s", __func__, version);

	if (!steamuser_version)
		steamuser_version = version;
}

struct ISteamUser *SteamUser(void)
{
    LOGTRACE(LCF_STEAM);

    if (!Global::shared_config.virtual_steam) {
        LINK_NAMESPACE(SteamUser, "steam_api");
        return orig::SteamUser();
    }

	static struct ISteamUser *cached_iface = nullptr;

	if (!steamuser_version)
	{
		steamuser_version = STEAMUSER_INTERFACE_VERSION_023;
        LOG(LL_WARN, LCF_STEAM, "ISteamUser: No version specified, defaulting to %s", steamuser_version);
	}

	if (!cached_iface)
		cached_iface = SteamUser_generic(steamuser_version);

	return cached_iface;
}

HSteamUser ISteamUser_GetHSteamUser()
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

bool ISteamUser_BLoggedOn()
{
    LOGTRACE(LCF_STEAM);
    return false; // Required for N++ to pass the splash screen
}

CSteamID ISteamUser_GetSteamID()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

int ISteamUser_InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, unsigned int unIPServer, uint16_t usPortServer, bool bSecure )
{
    LOGTRACE(LCF_STEAM);

    if(!pAuthBlob || cbMaxAuthBlob < 1)
        return 0;

    *(char *)pAuthBlob = 0;
    return 1;
}

void ISteamUser_TerminateGameConnection( unsigned int unIPServer, uint16_t usPortServer )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamUser_TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamUser_GetUserDataFolder( char *pchBuffer, int cubBuffer )
{
    LOGTRACE(LCF_STEAM);
    strncpy(pchBuffer, steamuserdir, cubBuffer-1);
    LOG(LL_DEBUG, LCF_STEAM, "user data folder = \"%s\".", steamuserdir);
    return true;
}

void ISteamUser_StartVoiceRecording()
{
    LOGTRACE(LCF_STEAM);
}

void ISteamUser_StopVoiceRecording()
{
    LOGTRACE(LCF_STEAM);
}

EVoiceResult ISteamUser_GetAvailableVoice( unsigned int *pcbCompressed, unsigned int *pcbUncompressed_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated)
{
    LOGTRACE(LCF_STEAM);
    return 3; //k_EVoiceResultNoData
}

EVoiceResult ISteamUser_GetVoice( bool bWantCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated, unsigned int cbUncompressedDestBufferSize_Deprecated, unsigned int *nUncompressBytesWritten_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated)
{
    LOGTRACE(LCF_STEAM);
    return 3; //k_EVoiceResultNoData
}

EVoiceResult ISteamUser_DecompressVoice( const void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, unsigned int nDesiredSampleRate )
{
    LOGTRACE(LCF_STEAM);
    return 3; //k_EVoiceResultNoData
}

unsigned int ISteamUser_GetVoiceOptimalSampleRate()
{
    LOGTRACE(LCF_STEAM);
    return 44100;
}

HAuthTicket ISteamUser_GetAuthSessionTicket( void *pTicket, int cbMaxTicket, unsigned int *pcbTicket )
{
    LOGTRACE(LCF_STEAM);
    // if (pcbTicket)
    //     *pcbTicket = 8;
    return 1;
}

HAuthTicket ISteamUser_GetAuthTicketForWebApi( const char *pchIdentity )
{
    LOGTRACE(LCF_STEAM);
    // if (pcbTicket)
    //     *pcbTicket = 8;
    return 1;
}
    
EBeginAuthSessionResult ISteamUser_BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    LOGTRACE(LCF_STEAM);
    return 0; //k_EBeginAuthSessionResultOK;
}

void ISteamUser_EndAuthSession( CSteamID steamID )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamUser_CancelAuthTicket( HAuthTicket hAuthTicket )
{
    LOGTRACE(LCF_STEAM);
}

}
