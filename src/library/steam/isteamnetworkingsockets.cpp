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

#include "isteamnetworkingsockets.h"
#include "../logging.h"

namespace libtas {

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketIP( const SteamNetworkingIPAddr &localAddress, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectByIPAddress( const SteamNetworkingIPAddr &address, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketP2P( int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2P( const SteamNetworkingIdentity &identityRemote, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::AcceptConnection( HSteamNetConnection hConn )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::CloseConnection( HSteamNetConnection hPeer, int nReason, const char *pszDebug, bool bEnableLinger )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamNetworkingSockets::CloseListenSocket( HSteamListenSocket hSocket )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamNetworkingSockets::SetConnectionUserData( HSteamNetConnection hPeer, int64_t nUserData )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int64_t ISteamNetworkingSockets::GetConnectionUserData( HSteamNetConnection hPeer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

void ISteamNetworkingSockets::SetConnectionName( HSteamNetConnection hPeer, const char *pszName )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamNetworkingSockets::GetConnectionName( HSteamNetConnection hPeer, char *pszName, int nMaxLen )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32_t cbData, int nSendFlags, int64_t *pOutMessageNumber )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamNetworkingSockets::SendMessages( int nMessages, SteamNetworkingMessage_t *const *pMessages, int64_t *pOutMessageNumberOrResult )
{
    DEBUGLOGCALL(LCF_STEAM);
}

EResult ISteamNetworkingSockets::FlushMessagesOnConnection( HSteamNetConnection hConn )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingSockets::ReceiveMessagesOnConnection( HSteamNetConnection hConn, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::GetConnectionInfo( HSteamNetConnection hConn, SteamNetConnectionInfo_t *pInfo )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::GetConnectionRealTimeStatus( HSteamNetConnection hConn, SteamNetConnectionRealTimeStatus_t *pStatus,
		int nLanes, SteamNetConnectionRealTimeLaneStatus_t *pLanes )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingSockets::GetDetailedConnectionStatus( HSteamNetConnection hConn, char *pszBuf, int cbBuf )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

bool ISteamNetworkingSockets::GetListenSocketAddress( HSteamListenSocket hSocket, SteamNetworkingIPAddr *address )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::CreateSocketPair( HSteamNetConnection *pOutConnection1, HSteamNetConnection *pOutConnection2, bool bUseNetworkLoopback, const SteamNetworkingIdentity *pIdentity1, const SteamNetworkingIdentity *pIdentity2 )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::ConfigureConnectionLanes( HSteamNetConnection hConn, int nNumLanes, const int *pLanePriorities, const uint16_t *pLaneWeights )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::GetIdentity( SteamNetworkingIdentity *pIdentity )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

ESteamNetworkingAvailability ISteamNetworkingSockets::InitAuthentication()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

ESteamNetworkingAvailability ISteamNetworkingSockets::GetAuthenticationStatus( SteamNetAuthenticationStatus_t *pDetails )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamNetPollGroup ISteamNetworkingSockets::CreatePollGroup()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::DestroyPollGroup( HSteamNetPollGroup hPollGroup )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::SetConnectionPollGroup( HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamNetworkingSockets::ReceiveMessagesOnPollGroup( HSteamNetPollGroup hPollGroup, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::ReceivedRelayAuthTicket( const void *pvTicket, int cbTicket, SteamDatagramRelayAuthTicket *pOutParsedTicket )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamNetworkingSockets::FindRelayAuthTicketForServer( const SteamNetworkingIdentity &identityGameServer, int nRemoteVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectToHostedDedicatedServer( const SteamNetworkingIdentity &identityTarget, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

uint16_t ISteamNetworkingSockets::GetHostedDedicatedServerPort()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

SteamNetworkingPOPID ISteamNetworkingSockets::GetHostedDedicatedServerPOPID()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetHostedDedicatedServerAddress( SteamDatagramHostedAddress *pRouting )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamListenSocket ISteamNetworkingSockets::CreateHostedDedicatedServerListenSocket( int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetGameCoordinatorServerLogin( SteamDatagramGameCoordinatorServerLogin *pLoginInfo, int *pcbSignedBlob, void *pBlob )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2PCustomSignaling( ISteamNetworkingConnectionSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::ReceivedP2PCustomSignal( const void *pMsg, int cbMsg, ISteamNetworkingSignalingRecvContext *pContext )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::GetCertificateRequest( int *pcbBlob, void *pBlob, SteamNetworkingErrMsg &errMsg )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::SetCertificate( const void *pCertificate, int cbCertificate, SteamNetworkingErrMsg &errMsg )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamNetworkingSockets::ResetIdentity( const SteamNetworkingIdentity *pIdentity )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamNetworkingSockets::RunCallbacks()
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamNetworkingSockets::BeginAsyncRequestFakeIP( int nNumPorts )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamNetworkingSockets::GetFakeIP( int idxFirstPort, SteamNetworkingFakeIPResult_t *pInfo )
{
    DEBUGLOGCALL(LCF_STEAM);
}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketP2PFakeIP( int idxFakePort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetRemoteFakeIPForConnection( HSteamNetConnection hConn, SteamNetworkingIPAddr *pOutAddr )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

ISteamNetworkingFakeUDPPort *ISteamNetworkingSockets::CreateFakeUDPPort( int idxFakeServerPort )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

}
