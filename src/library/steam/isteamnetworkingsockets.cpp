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

#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace libtas {

namespace {

struct ConnectionState {
    int64_t userData = -1;
    std::string name;
    HSteamNetPollGroup pollGroup = 0;
};

std::mutex networkStateMutex;
HSteamListenSocket nextListenSocket = 1;
HSteamNetConnection nextConnection = 1;
HSteamNetPollGroup nextPollGroup = 1;
std::unordered_set<HSteamListenSocket> listenSockets;
std::unordered_map<HSteamNetConnection, ConnectionState> connections;
std::unordered_set<HSteamNetPollGroup> pollGroups;

HSteamListenSocket createListenSocketHandle()
{
    std::lock_guard<std::mutex> lock(networkStateMutex);
    HSteamListenSocket handle = nextListenSocket++;
    listenSockets.insert(handle);
    return handle;
}

HSteamNetConnection createConnectionHandle()
{
    std::lock_guard<std::mutex> lock(networkStateMutex);
    HSteamNetConnection handle = nextConnection++;
    connections.emplace(handle, ConnectionState{});
    return handle;
}

HSteamNetPollGroup createPollGroupHandle()
{
    std::lock_guard<std::mutex> lock(networkStateMutex);
    HSteamNetPollGroup handle = nextPollGroup++;
    pollGroups.insert(handle);
    return handle;
}

bool hasConnection(HSteamNetConnection handle)
{
    return handle > 0 && connections.find(handle) != connections.end();
}

}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketIP( const SteamNetworkingIPAddr &localAddress, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return createListenSocketHandle();
}

HSteamNetConnection ISteamNetworkingSockets::ConnectByIPAddress( const SteamNetworkingIPAddr &address, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return createConnectionHandle();
}

HSteamListenSocket ISteamNetworkingSockets::CreateListenSocketP2P( int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return createListenSocketHandle();
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2P( const SteamNetworkingIdentity &identityRemote, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return createConnectionHandle();
}

EResult ISteamNetworkingSockets::AcceptConnection( HSteamNetConnection hConn )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    return hasConnection(hConn) ? 1 : 0;
}

bool ISteamNetworkingSockets::CloseConnection( HSteamNetConnection hPeer, int nReason, const char *pszDebug, bool bEnableLinger )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    return connections.erase(hPeer) > 0;
}

bool ISteamNetworkingSockets::CloseListenSocket( HSteamListenSocket hSocket )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    return listenSockets.erase(hSocket) > 0;
}

bool ISteamNetworkingSockets::SetConnectionUserData( HSteamNetConnection hPeer, int64_t nUserData )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    auto it = connections.find(hPeer);
    if (it == connections.end())
        return false;

    it->second.userData = nUserData;
    return true;
}

int64_t ISteamNetworkingSockets::GetConnectionUserData( HSteamNetConnection hPeer )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    auto it = connections.find(hPeer);
    if (it == connections.end())
        return -1;

    return it->second.userData;
    return -1;
}

void ISteamNetworkingSockets::SetConnectionName( HSteamNetConnection hPeer, const char *pszName )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    auto it = connections.find(hPeer);
    if (it == connections.end())
        return;

    it->second.name = pszName ? pszName : "";
}

bool ISteamNetworkingSockets::GetConnectionName( HSteamNetConnection hPeer, char *pszName, int nMaxLen )
{
    LOGTRACE(LCF_STEAM);
    if (!pszName || nMaxLen <= 0)
        return false;

    std::lock_guard<std::mutex> lock(networkStateMutex);
    auto it = connections.find(hPeer);
    if (it == connections.end()) {
        pszName[0] = '\0';
        return false;
    }

    std::strncpy(pszName, it->second.name.c_str(), nMaxLen - 1);
    pszName[nMaxLen - 1] = '\0';
    return true;
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
    if (!pOutConnection1 || !pOutConnection2)
        return false;

    *pOutConnection1 = createConnectionHandle();
    *pOutConnection2 = createConnectionHandle();
    return true;
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
    return createPollGroupHandle();
}

bool ISteamNetworkingSockets::DestroyPollGroup( HSteamNetPollGroup hPollGroup )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    if (pollGroups.erase(hPollGroup) == 0)
        return false;

    for (auto &connection : connections) {
        if (connection.second.pollGroup == hPollGroup)
            connection.second.pollGroup = 0;
    }
    return true;
}

bool ISteamNetworkingSockets::SetConnectionPollGroup( HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(networkStateMutex);
    auto connectionIt = connections.find(hConn);
    if (connectionIt == connections.end())
        return false;

    if (hPollGroup != 0 && pollGroups.find(hPollGroup) == pollGroups.end())
        return false;

    connectionIt->second.pollGroup = hPollGroup;
    return true;
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
    return createConnectionHandle();
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
    return createListenSocketHandle();
}

EResult ISteamNetworkingSockets::GetGameCoordinatorServerLogin( SteamDatagramGameCoordinatorServerLogin *pLoginInfo, int *pcbSignedBlob, void *pBlob )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

HSteamNetConnection ISteamNetworkingSockets::ConnectP2PCustomSignaling( ISteamNetworkingConnectionSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
{
    LOGTRACE(LCF_STEAM);
    return createConnectionHandle();
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
    return createListenSocketHandle();
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
