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

#include "isteamnetworkingutils.h"
#include "../logging.h"

namespace libtas {

SteamNetworkingMessage_t *ISteamNetworkingUtils::AllocateMessage( int cbAllocateBuffer )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

ESteamNetworkingAvailability ISteamNetworkingUtils::GetRelayNetworkStatus( SteamRelayNetworkStatus_t *pDetails )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

float ISteamNetworkingUtils::GetLocalPingLocation( SteamNetworkPingLocation_t &result )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0.0f;
}

int ISteamNetworkingUtils::EstimatePingTimeBetweenTwoLocations( const SteamNetworkPingLocation_t &location1, const SteamNetworkPingLocation_t &location2 )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingUtils::EstimatePingTimeFromLocalHost( const SteamNetworkPingLocation_t &remoteLocation )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamNetworkingUtils::ConvertPingLocationToString( const SteamNetworkPingLocation_t &location, char *pszBuf, int cchBufSize )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamNetworkingUtils::ParsePingLocationString( const char *pszString, SteamNetworkPingLocation_t &result )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

bool ISteamNetworkingUtils::CheckPingDataUpToDate( float flMaxAgeSeconds )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

int ISteamNetworkingUtils::GetPingToDataCenter( SteamNetworkingPOPID popID, SteamNetworkingPOPID *pViaRelayPoP )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingUtils::GetDirectPingToPOP( SteamNetworkingPOPID popID )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingUtils::GetPOPCount()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

int ISteamNetworkingUtils::GetPOPList( SteamNetworkingPOPID *list, int nListSz )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

SteamNetworkingMicroseconds ISteamNetworkingUtils::GetLocalTimestamp()
{
    DEBUGLOGCALL(LCF_STEAM);
    return 24*3600*30*1e6;
}

void ISteamNetworkingUtils::SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    DEBUGLOGCALL(LCF_STEAM);
}

ESteamNetworkingFakeIPType ISteamNetworkingUtils::GetIPv4FakeIPType( uint32_t nIPv4 )
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

EResult ISteamNetworkingUtils::GetRealIdentityForFakeIP( const SteamNetworkingIPAddr &fakeIP, SteamNetworkingIdentity *pOutRealIdentity )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

bool ISteamNetworkingUtils::SetConfigValue( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj,
		ESteamNetworkingConfigDataType eDataType, const void *pArg )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

ESteamNetworkingGetConfigValueResult ISteamNetworkingUtils::GetConfigValue( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj,
		ESteamNetworkingConfigDataType *pOutDataType, void *pResult, size_t *cbResult )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

const char *ISteamNetworkingUtils::GetConfigValueInfo( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigDataType *pOutDataType,
		ESteamNetworkingConfigScope *pOutScope )
{
    DEBUGLOGCALL(LCF_STEAM);
    return nullptr;
}

ESteamNetworkingConfigValue ISteamNetworkingUtils::IterateGenericEditableConfigValues( ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamNetworkingUtils::SteamNetworkingIPAddr_ToString( const SteamNetworkingIPAddr &addr, char *buf, size_t cbBuf, bool bWithPort )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamNetworkingUtils::SteamNetworkingIPAddr_ParseString( SteamNetworkingIPAddr *pAddr, const char *pszStr )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

ESteamNetworkingFakeIPType ISteamNetworkingUtils::SteamNetworkingIPAddr_GetFakeIPType( const SteamNetworkingIPAddr &addr )
{
    DEBUGLOGCALL(LCF_STEAM);
    return 0;
}

void ISteamNetworkingUtils::SteamNetworkingIdentity_ToString( const SteamNetworkingIdentity &identity, char *buf, size_t cbBuf )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamNetworkingUtils::SteamNetworkingIdentity_ParseString( SteamNetworkingIdentity *pIdentity, const char *pszStr )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

}
