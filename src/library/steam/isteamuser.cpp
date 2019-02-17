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

// FIXME used for getcwd, replace me with a dir from the config --GM
#include <unistd.h>

#include "isteamuser.h"
#include "../logging.h"

namespace libtas {

HSteamUser ISteamUser::GetHSteamUser()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamUser::BLoggedOn()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

CSteamID ISteamUser::GetSteamID()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

int ISteamUser::InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, unsigned int unIPServer, uint16_t usPortServer, bool bSecure )
{
    DEBUGLOGCALL(LCF_STEAM);

    if(!pAuthBlob || cbMaxAuthBlob < 1)
        return 0;

    *(char *)pAuthBlob = 0;
    return 1;
}

void ISteamUser::TerminateGameConnection( unsigned int unIPServer, uint16_t usPortServer )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamUser::TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamUser::GetUserDataFolder( char *pchBuffer, int cubBuffer )
{
    // FIXME replace me with a dir from the config --GM
    char cwdbuf[2048];
    DEBUGLOGCALL(LCF_STEAM);
    debuglogstdio(LCF_STEAM, "user data folder, cwd = \"%s\".", getcwd(cwdbuf, sizeof(cwdbuf)-1));
    return true;
}

void ISteamUser::StartVoiceRecording()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamUser::StopVoiceRecording()
{
    DEBUGLOGCALL(LCF_STEAM);
}

}
