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

#include "isteamnetworkingsockets.h"

#include "logging.h"

namespace libtas {

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketIP( const SteamNetworkingIPAddr &localAddress, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectByIPAddress( const SteamNetworkingIPAddr &address, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketP2P( int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2P( const SteamNetworkingIdentity &identityRemote, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::AcceptConnection( HSteamNetConnection hConn )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::CloseConnection( HSteamNetConnection hPeer, int nReason, const char *pszDebug, bool bEnableLinger )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworkingSockets::CloseListenSocket( HSteamListenSocket hSocket )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamNetworkingSockets::SetConnectionUserData( HSteamNetConnection hPeer, int64_t nUserData )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int64_t ISteamNetworkingSockets::GetConnectionUserData( HSteamNetConnection hPeer )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

void ISteamNetworkingSockets::SetConnectionName( HSteamNetConnection hPeer, const char *pszName )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamNetworkingSockets::GetConnectionName( HSteamNetConnection hPeer, char *pszName, int nMaxLen )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32_t cbData, int nSendFlags, int64_t *pOutMessageNumber )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamNetworkingSockets::SendMessages( int nMessages, SteamNetworkingMessage_t *const *pMessages, int64_t *pOutMessageNumberOrResult )
{
    LOGTRACE(LCF_STEAM);
}

EResult ISteamNetworkingSockets::FlushMessagesOnConnection( HSteamNetConnection hConn )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamNetworkingSockets::ReceiveMessagesOnConnection( HSteamNetConnection hConn, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::GetConnectionInfo( HSteamNetConnection hConn, SteamNetConnectionInfo_t *pInfo )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::GetConnectionRealTimeStatus( HSteamNetConnection hConn, SteamNetConnectionRealTimeStatus_t *pStatus,
		int nLanes, SteamNetConnectionRealTimeLaneStatus_t *pLanes )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamNetworkingSockets::GetDetailedConnectionStatus( HSteamNetConnection hConn, char *pszBuf, int cbBuf )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

bool ISteamNetworkingSockets::GetListenSocketAddress( HSteamListenSocket hSocket, SteamNetworkingIPAddr *address )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::CreateSocketPair( HSteamNetConnection *pOutConnection1, HSteamNetConnection *pOutConnection2, bool bUseNetworkLoopback, const SteamNetworkingIdentity *pIdentity1, const SteamNetworkingIdentity *pIdentity2 )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

EResult ISteamNetworkingSockets::ConfigureConnectionLanes( HSteamNetConnection hConn, int nNumLanes, const int *pLanePriorities, const uint16_t *pLaneWeights )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::GetIdentity( SteamNetworkingIdentity *pIdentity )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

ESteamNetworkingAvailability ISteamNetworkingSockets::InitAuthentication()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ESteamNetworkingAvailability ISteamNetworkingSockets::GetAuthenticationStatus( SteamNetAuthenticationStatus_t *pDetails )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetPollGroup ISteamNetworkingSockets::CreatePollGroup()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::DestroyPollGroup( HSteamNetPollGroup hPollGroup )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::SetConnectionPollGroup( HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamNetworkingSockets::ReceiveMessagesOnPollGroup( HSteamNetPollGroup hPollGroup, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::ReceivedRelayAuthTicket( const void *pvTicket, int cbTicket, SteamDatagramRelayAuthTicket *pOutParsedTicket )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamNetworkingSockets::FindRelayAuthTicketForServer( const SteamNetworkingIdentity &identityGameServer, int nRemoteVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectToHostedDedicatedServer( const SteamNetworkingIdentity &identityTarget, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

uint16_t ISteamNetworkingSockets::GetHostedDedicatedServerPort()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamNetworkingPOPID ISteamNetworkingSockets::GetHostedDedicatedServerPOPID()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetHostedDedicatedServerAddress( SteamDatagramHostedAddress *pRouting )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamListenSocket ISteamNetworkingSockets::CreateHostedDedicatedServerListenSocket( int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetGameCoordinatorServerLogin( SteamDatagramGameCoordinatorServerLogin *pLoginInfo, int *pcbSignedBlob, void *pBlob )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2PCustomSignaling( ISteamNetworkingConnectionSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingSockets::ReceivedP2PCustomSignal( const void *pMsg, int cbMsg, ISteamNetworkingSignalingRecvContext *pContext )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::GetCertificateRequest( int *pcbBlob, void *pBlob, SteamNetworkingErrMsg &errMsg )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamNetworkingSockets::SetCertificate( const void *pCertificate, int cbCertificate, SteamNetworkingErrMsg &errMsg )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamNetworkingSockets::ResetIdentity( const SteamNetworkingIdentity *pIdentity )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamNetworkingSockets::RunCallbacks()
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamNetworkingSockets::BeginAsyncRequestFakeIP( int nNumPorts )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamNetworkingSockets::GetFakeIP( int idxFirstPort, SteamNetworkingFakeIPResult_t *pInfo )
{
    LOGTRACE(LCF_STEAM);
}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketP2PFakeIP( int idxFakePort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EResult ISteamNetworkingSockets::GetRemoteFakeIPForConnection( HSteamNetConnection hConn, SteamNetworkingIPAddr *pOutAddr )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

ISteamNetworkingFakeUDPPort *ISteamNetworkingSockets::CreateFakeUDPPort( int idxFakeServerPort )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

}
