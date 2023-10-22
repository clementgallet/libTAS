/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "isteamnetworkingmessages.h"
#include "../logging.h"

namespace libtas {

EResult ISteamNetworkingMessages::SendMessageToUser( const SteamNetworkingIdentity &identityRemote, const void *pubData, uint32_t cubData, int nSendFlags, int nRemoteChannel )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingMessages::ReceiveMessagesOnChannel( int nLocalChannel, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingMessages::AcceptSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingMessages::CloseSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingMessages::CloseChannelWithUser( const SteamNetworkingIdentity &identityRemote, int nLocalChannel )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

ESteamNetworkingConnectionState ISteamNetworkingMessages::GetSessionConnectionInfo( const SteamNetworkingIdentity &identityRemote, SteamNetConnectionInfo_t *pConnectionInfo, SteamNetConnectionRealTimeStatus_t *pQuickStatus )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

}
