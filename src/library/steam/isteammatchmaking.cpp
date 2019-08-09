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

#include "isteammatchmaking.h"
#include "../logging.h"

namespace libtas {

int ISteamMatchmaking::GetFavoriteGameCount()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::GetFavoriteGame( int iGame, AppId_t *pnAppID, uint32_t *pnIP, uint16_t *pnConnPort, uint16_t *pnQueryPort, uint32_t *punFlags, uint32_t *pRTime32LastPlayedOnServer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamMatchmaking::AddFavoriteGame( AppId_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags, uint32_t rTime32LastPlayedOnServer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::RemoveFavoriteGame( AppId_t nAppID, uint32_t nIP, uint16_t nConnPort, uint16_t nQueryPort, uint32_t unFlags )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

SteamAPICall_t ISteamMatchmaking::RequestLobbyList()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

void ISteamMatchmaking::AddRequestLobbyListStringFilter( const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListNearValueFilter( const char *pchKeyToMatch, int nValueToBeCloseTo )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListFilterSlotsAvailable( int nSlotsAvailable )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter eLobbyDistanceFilter )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListResultCountFilter( int cMaxResults )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmaking::AddRequestLobbyListCompatibleMembersFilter( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
}

CSteamID ISteamMatchmaking::GetLobbyByIndex( int iLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamMatchmaking::CreateLobby( ELobbyType eLobbyType, int cMaxMembers )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

SteamAPICall_t ISteamMatchmaking::JoinLobby( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 1;
}

void ISteamMatchmaking::LeaveLobby( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamMatchmaking::InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamMatchmaking::GetNumLobbyMembers( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

CSteamID ISteamMatchmaking::GetLobbyMemberByIndex( CSteamID steamIDLobby, int iMember )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

const char *ISteamMatchmaking::GetLobbyData( CSteamID steamIDLobby, const char *pchKey )
{
    DEBUGLOGCALL(LCF_STEAM);
    return "";
}

bool ISteamMatchmaking::SetLobbyData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyDataCount( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::GetLobbyDataByIndex( CSteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamMatchmaking::DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

const char *ISteamMatchmaking::GetLobbyMemberData( CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey )
{
    DEBUGLOGCALL(LCF_STEAM);
    return "";
}

void ISteamMatchmaking::SetLobbyMemberData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamMatchmaking::SendLobbyChatMsg( CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::RequestLobbyData( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamMatchmaking::SetLobbyGameServer( CSteamID steamIDLobby, uint32_t unGameServerIP, uint16_t unGameServerPort, CSteamID steamIDGameServer )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamMatchmaking::GetLobbyGameServer( CSteamID steamIDLobby, uint32_t *punGameServerIP, uint16_t *punGameServerPort, CSteamID *psteamIDGameServer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamMatchmaking::SetLobbyMemberLimit( CSteamID steamIDLobby, int cMaxMembers )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamMatchmaking::GetLobbyMemberLimit( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::SetLobbyType( CSteamID steamIDLobby, ELobbyType eLobbyType )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamMatchmaking::SetLobbyJoinable( CSteamID steamIDLobby, bool bLobbyJoinable )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

CSteamID ISteamMatchmaking::GetLobbyOwner( CSteamID steamIDLobby )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamMatchmaking::SetLobbyOwner( CSteamID steamIDLobby, CSteamID steamIDNewOwner )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamMatchmaking::SetLinkedLobby( CSteamID steamIDLobby, CSteamID steamIDLobbyDependent )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

HServerListRequest ISteamMatchmakingServers::RequestInternetServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestFriendsServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestFavoritesServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestHistoryServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

HServerListRequest ISteamMatchmakingServers::RequestSpectatorServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32_t nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

void ISteamMatchmakingServers::ReleaseRequest( HServerListRequest hServerListRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
}

gameserveritem_t *ISteamMatchmakingServers::GetServerDetails( HServerListRequest hRequest, int iServer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

void ISteamMatchmakingServers::CancelQuery( HServerListRequest hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamMatchmakingServers::RefreshQuery( HServerListRequest hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamMatchmakingServers::IsRefreshing( HServerListRequest hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamMatchmakingServers::GetServerCount( HServerListRequest hRequest )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamMatchmakingServers::RefreshServer( HServerListRequest hRequest, int iServer )
{
    DEBUGLOGCALL(LCF_STEAM);
}

HServerQuery ISteamMatchmakingServers::PingServer( uint32_t unIP, uint16_t usPort, ISteamMatchmakingPingResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

HServerQuery ISteamMatchmakingServers::PlayerDetails( uint32_t unIP, uint16_t usPort, ISteamMatchmakingPlayersResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

HServerQuery ISteamMatchmakingServers::ServerRules( uint32_t unIP, uint16_t usPort, ISteamMatchmakingRulesResponse *pRequestServersResponse )
{
    DEBUGLOGCALL(LCF_STEAM);
    return -1;
}

void ISteamMatchmakingServers::CancelServerQuery( HServerQuery hServerQuery )
{
    DEBUGLOGCALL(LCF_STEAM);
}

}
