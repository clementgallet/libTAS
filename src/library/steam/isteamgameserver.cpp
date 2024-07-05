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

#include "isteamgameserver.h"

#include "logging.h"

namespace libtas {

bool ISteamGameServer::InitGameServer( uint32_t unIP, uint16_t usGamePort, uint16_t usQueryPort, uint32_t unFlags, AppId_t nGameAppId, const char *pchVersionString )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamGameServer::SetProduct( const char *pszProduct )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetGameDescription( const char *pszGameDescription )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetModDir( const char *pszModDir )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetDedicatedServer( bool bDedicated )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::LogOn( const char *pszToken )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::LogOnAnonymous()
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::LogOff()
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamGameServer::BLoggedOn()
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamGameServer::BSecure()
{
    LOGTRACE(LCF_STEAM);
    return false;
}

CSteamID ISteamGameServer::GetSteamID()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::WasRestartRequested()
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamGameServer::SetMaxPlayerCount( int cPlayersMax )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetBotPlayerCount( int cBotplayers )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetServerName( const char *pszServerName )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetMapName( const char *pszMapName )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetPasswordProtected( bool bPasswordProtected )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetSpectatorPort( uint16_t unSpectatorPort )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetSpectatorServerName( const char *pszSpectatorServerName )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::ClearAllKeyValues()
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetKeyValue( const char *pKey, const char *pValue )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetGameTags( const char *pchGameTags )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetGameData( const char *pchGameData )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetRegion( const char *pszRegion )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamGameServer::SendUserConnectAndAuthenticate( uint32_t unIPClient, const void *pvAuthBlob, uint32_t cubAuthBlobSize, CSteamID *pSteamIDUser )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

CSteamID ISteamGameServer::CreateUnauthenticatedUserConnection()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamGameServer::SendUserDisconnect( CSteamID steamIDUser )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamGameServer::BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32_t uScore )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

HAuthTicket ISteamGameServer::GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32_t *pcbTicket )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EBeginAuthSessionResult ISteamGameServer::BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamGameServer::EndAuthSession( CSteamID steamID )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::CancelAuthTicket( HAuthTicket hAuthTicket )
{
    LOGTRACE(LCF_STEAM);
}

EUserHasLicenseForAppResult ISteamGameServer::UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

void ISteamGameServer::GetGameplayStats( )
{
    LOGTRACE(LCF_STEAM);
}

SteamAPICall_t ISteamGameServer::GetServerReputation()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

uint32_t ISteamGameServer::GetPublicIP()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamGameServer::HandleIncomingPacket( const void *pData, int cbData, uint32_t srcIP, uint16_t srcPort )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamGameServer::GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32_t *pNetAdr, uint16_t *pPort )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamGameServer::EnableHeartbeats( bool bActive )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::SetHeartbeatInterval( int iHeartbeatInterval )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamGameServer::ForceHeartbeat()
{
    LOGTRACE(LCF_STEAM);
}

SteamAPICall_t ISteamGameServer::AssociateWithClan( CSteamID steamIDClan )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamGameServer::ComputeNewPlayerCompatibility( CSteamID steamIDNewPlayer )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

}
