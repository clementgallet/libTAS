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

#include "isteamfriends.h"
#include "../logging.h"

namespace libtas {

const char *ISteamFriends::GetPersonaName()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

SteamAPICall_t ISteamFriends::SetPersonaName( const char *pchPersonaName )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

EPersonaState ISteamFriends::GetPersonaState()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0; // k_EPersonaStateOffline
}

int ISteamFriends::GetFriendCount( int iFriendFlags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

CSteamID ISteamFriends::GetFriendByIndex( int iFriend, int iFriendFlags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

EFriendRelationship ISteamFriends::GetFriendRelationship( CSteamID steamIDFriend )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0; // k_EFriendRelationshipNone
}

EPersonaState ISteamFriends::GetFriendPersonaState( CSteamID steamIDFriend )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0; // k_EPersonaStateOffline
}

const char *ISteamFriends::GetFriendPersonaName( CSteamID steamIDFriend )
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

bool ISteamFriends::GetFriendGamePlayed( CSteamID steamIDFriend, FriendGameInfo_t *pFriendGameInfo )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

const char *ISteamFriends::GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName )
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

int ISteamFriends::GetFriendSteamLevel( CSteamID steamIDFriend )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

const char *ISteamFriends::GetPlayerNickname( CSteamID steamIDPlayer )
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

}
