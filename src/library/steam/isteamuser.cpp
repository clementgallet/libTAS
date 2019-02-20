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

#include "isteamuser.h"
#include "../logging.h"

namespace libtas {

char steamuserdir[2048] = "/NOTVALID";

// FIXME: prototype for this function should be declared in a header somewhere
// (used by library/main.cpp)
void SteamUser_SetUserDataFolder(std::string path)
{
    DEBUGLOGCALL(LCF_STEAM);
    strncpy(steamuserdir, path.c_str(), sizeof(steamuserdir)-1);
}

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
    DEBUGLOGCALL(LCF_STEAM);
    strncpy(pchBuffer, steamuserdir, cubBuffer-1);
    debuglogstdio(LCF_STEAM, "user data folder = \"%s\".", steamuserdir);
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
