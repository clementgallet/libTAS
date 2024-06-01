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

#include "isteammatchmaking.h"

#include "logging.h"

namespace libtas {

int ISteamMatchmaking::GetFavoriteGameCount()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::GetFavoriteGame( int iGame, AppId_t *pnAppID, uint32_t *pnIP, uint16_t *pnConnPort, uint16_t *pnQueryPort, uint32_t *punFlags, uint32_t *pRTime32LastPlayedOnServer )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamMatchmaking::AddFavoriteGame( AppId_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags, uint32_t rTime32LastPlayedOnServer )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::RemoveFavoriteGame( AppId_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamMatchmaking::RequestLobbyList()
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

void ISteamMatchmaking::AddRequestLobbyListStringFilter( const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListNearValueFilter( const char *pchKeyToMatch, int nValueToBeCloseTo )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListFilterSlotsAvailable( int nSlotsAvailable )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter eLobbyDistanceFilter )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListResultCountFilter( int cMaxResults )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListCompatibleMembersFilter( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
}

CSteamID ISteamMatchmaking::GetLobbyByIndex( int iLobby )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamMatchmaking::CreateLobby( ELobbyType eLobbyType, int cMaxMembers )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamMatchmaking::JoinLobby( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return 1;
}

void ISteamMatchmaking::LeaveLobby( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamMatchmaking::InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamMatchmaking::GetNumLobbyMembers( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

CSteamID ISteamMatchmaking::GetLobbyMemberByIndex( CSteamID steamIDLobby, int iMember )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

const char *ISteamMatchmaking::GetLobbyData( CSteamID steamIDLobby, const char *pchKey )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

bool ISteamMatchmaking::SetLobbyData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyDataCount( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::GetLobbyDataByIndex( CSteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamMatchmaking::DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

const char *ISteamMatchmaking::GetLobbyMemberData( CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

void ISteamMatchmaking::SetLobbyMemberData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamMatchmaking::SendLobbyChatMsg( CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::RequestLobbyData( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamMatchmaking::SetLobbyGameServer( CSteamID steamIDLobby, uint32_t unGameServerIP, uint16_t unGameServerPort, CSteamID steamIDGameServer )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamMatchmaking::GetLobbyGameServer( CSteamID steamIDLobby, uint32_t *punGameServerIP, uint16_t *punGameServerPort, CSteamID *psteamIDGameServer )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamMatchmaking::SetLobbyMemberLimit( CSteamID steamIDLobby, int cMaxMembers )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyMemberLimit( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::SetLobbyType( CSteamID steamIDLobby, ELobbyType eLobbyType )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamMatchmaking::SetLobbyJoinable( CSteamID steamIDLobby, bool bLobbyJoinable )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

CSteamID ISteamMatchmaking::GetLobbyOwner( CSteamID steamIDLobby )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::SetLobbyOwner( CSteamID steamIDLobby, CSteamID steamIDNewOwner )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamMatchmaking::SetLinkedLobby( CSteamID steamIDLobby, CSteamID steamIDLobbyDependent )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

HServerListRequest ISteamMatchmakingServers::RequestInternetServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestFriendsServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestFavoritesServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestHistoryServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestSpectatorServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

void ISteamMatchmakingServers::ReleaseRequest( HServerListRequest hServerListRequest )
{
    LOGTRACE(LCF_STEAM);
}

gameserveritem_t *ISteamMatchmakingServers::GetServerDetails( HServerListRequest hRequest, int iServer )
{
    LOGTRACE(LCF_STEAM);
    return nullptr;
}

void ISteamMatchmakingServers::CancelQuery( HServerListRequest hRequest )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamMatchmakingServers::RefreshQuery( HServerListRequest hRequest )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamMatchmakingServers::IsRefreshing( HServerListRequest hRequest )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamMatchmakingServers::GetServerCount( HServerListRequest hRequest )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamMatchmakingServers::RefreshServer( HServerListRequest hRequest, int iServer )
{
    LOGTRACE(LCF_STEAM);
}

HServerQuery ISteamMatchmakingServers::PingServer( uint32_t unIP, uint16_t usPort, ISteamMatchmakingPingResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

HServerQuery ISteamMatchmakingServers::PlayerDetails( uint32_t unIP, uint16_t usPort, ISteamMatchmakingPlayersResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

HServerQuery ISteamMatchmakingServers::ServerRules( uint32_t unIP, uint16_t usPort, ISteamMatchmakingRulesResponse *pRequestServersResponse )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}

void ISteamMatchmakingServers::CancelServerQuery( HServerQuery hServerQuery )
{
    LOGTRACE(LCF_STEAM);
}

}
