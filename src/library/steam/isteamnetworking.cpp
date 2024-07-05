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

#include "isteamnetworking.h"

#include "logging.h"

namespace libtas {

bool ISteamNetworking::SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32_t cubData, EP2PSend eP2PSendType, int nChannel )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworking::IsP2PPacketAvailable( uint32_t *pcubMsgSize, int nChannel )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::ReadP2PPacket( void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::AcceptP2PSessionWithUser( CSteamID steamIDRemote )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::CloseP2PSessionWithUser( CSteamID steamIDRemote )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworking::CloseP2PChannelWithUser( CSteamID steamIDRemote, int nChannel )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworking::GetP2PSessionState( CSteamID steamIDRemote, P2PSessionState_t *pConnectionState )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::AllowP2PPacketRelay( bool bAllow )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SNetListenSocket_t ISteamNetworking::CreateListenSocket( int nVirtualP2PPort, uint32_t nIP, uint16_t nPort, bool bAllowUseOfPacketRelay )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SNetSocket_t ISteamNetworking::CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SNetSocket_t ISteamNetworking::CreateConnectionSocket( uint32_t nIP, uint16_t nPort, int nTimeoutSec )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworking::DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworking::DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworking::SendDataOnSocket( SNetSocket_t hSocket, void *pubData, uint32_t cubData, bool bReliable )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::IsDataAvailableOnSocket( SNetSocket_t hSocket, uint32_t *pcubMsgSize )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::RetrieveDataFromSocket( SNetSocket_t hSocket, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::IsDataAvailable( SNetListenSocket_t hListenSocket, uint32_t *pcubMsgSize, SNetSocket_t *phSocket )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::RetrieveData( SNetListenSocket_t hListenSocket, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, SNetSocket_t *phSocket )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32_t *punIPRemote, uint16_t *punPortRemote )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworking::GetListenSocketInfo( SNetListenSocket_t hListenSocket, uint32_t *pnIP, uint16_t *pnPort )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

ESNetSocketConnectionType ISteamNetworking::GetSocketConnectionType( SNetSocket_t hSocket )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamNetworking::GetMaxPacketSize( SNetSocket_t hSocket )
{
    LOGTRACE(LCF_STEAM);
    return 4096;
}

}
