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

#include "isteamgameserver.h"
#include "../logging.h"

namespace libtas {

bool ISteamGameServer::InitGameServer( uint32_t unIP, uint16_t usGamePort, uint16_t usQueryPort, uint32_t unFlags, AppId_t nGameAppId, const char *pchVersionString )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamGameServer::SetProduct( const char *pszProduct )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetGameDescription( const char *pszGameDescription )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetModDir( const char *pszModDir )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetDedicatedServer( bool bDedicated )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::LogOn( const char *pszToken )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::LogOnAnonymous()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::LogOff()
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamGameServer::BLoggedOn()
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

bool ISteamGameServer::BSecure()
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

CSteamID ISteamGameServer::GetSteamID()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::WasRestartRequested()
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamGameServer::SetMaxPlayerCount( int cPlayersMax )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetBotPlayerCount( int cBotplayers )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetServerName( const char *pszServerName )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetMapName( const char *pszMapName )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetPasswordProtected( bool bPasswordProtected )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetSpectatorPort( uint16_t unSpectatorPort )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetSpectatorServerName( const char *pszSpectatorServerName )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::ClearAllKeyValues()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetKeyValue( const char *pKey, const char *pValue )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetGameTags( const char *pchGameTags )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetGameData( const char *pchGameData )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetRegion( const char *pszRegion )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamGameServer::SendUserConnectAndAuthenticate( uint32_t unIPClient, const void *pvAuthBlob, uint32_t cubAuthBlobSize, CSteamID *pSteamIDUser )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

CSteamID ISteamGameServer::CreateUnauthenticatedUserConnection()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamGameServer::SendUserDisconnect( CSteamID steamIDUser )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamGameServer::BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32_t uScore )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

HAuthTicket ISteamGameServer::GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32_t *pcbTicket )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

EBeginAuthSessionResult ISteamGameServer::BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamGameServer::EndAuthSession( CSteamID steamID )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::CancelAuthTicket( HAuthTicket hAuthTicket )
{
    DEBUGLOGCALL(LCF_STEAM);
}

EUserHasLicenseForAppResult ISteamGameServer::UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamGameServer::GetGameplayStats( )
{
    DEBUGLOGCALL(LCF_STEAM);
}

SteamAPICall_t ISteamGameServer::GetServerReputation()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

uint32_t ISteamGameServer::GetPublicIP()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::HandleIncomingPacket( const void *pData, int cbData, uint32_t srcIP, uint16_t srcPort )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

int ISteamGameServer::GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32_t *pNetAdr, uint16_t *pPort )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamGameServer::EnableHeartbeats( bool bActive )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::SetHeartbeatInterval( int iHeartbeatInterval )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamGameServer::ForceHeartbeat()
{
    DEBUGLOGCALL(LCF_STEAM);
}

SteamAPICall_t ISteamGameServer::AssociateWithClan( CSteamID steamIDClan )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamGameServer::ComputeNewPlayerCompatibility( CSteamID steamIDNewPlayer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

}
