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

#include "isteamapps.h"
#include "../logging.h"

namespace libtas {

bool ISteamApps::BIsSubscribed()
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsLowViolence()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsCybercafe()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsVACBanned()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

const char *ISteamApps::GetCurrentGameLanguage()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "english";
}

const char *ISteamApps::GetAvailableGameLanguages()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "english";
}

bool ISteamApps::BIsSubscribedApp( AppId_t appID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsDlcInstalled( AppId_t appID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetEarliestPurchaseUnixTime( AppId_t nAppID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

bool ISteamApps::BIsSubscribedFromFreeWeekend()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

int ISteamApps::GetDLCCount()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

bool ISteamApps::BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

void ISteamApps::InstallDLC( AppId_t nAppID )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamApps::UninstallDLC( AppId_t nAppID )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamApps::RequestAppProofOfPurchaseKey( AppId_t nAppID )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamApps::GetCurrentBetaName( char *pchName, int cchNameBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
    strncpy(pchName, "public", cchNameBufferSize);
	return true;
}

bool ISteamApps::MarkContentCorrupt( bool bMissingFilesOnly )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, unsigned int cMaxDepots )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

unsigned int ISteamApps::GetAppInstallDir( AppId_t appID, char *pchFolder, unsigned int cchFolderBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
	return 0;
}

bool ISteamApps::BIsAppInstalled( AppId_t appID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

CSteamID ISteamApps::GetAppOwner()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

const char *ISteamApps::GetLaunchQueryParam( const char *pchKey )
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

bool ISteamApps::GetDlcDownloadProgress( AppId_t nAppID, uint64_t *punBytesDownloaded, uint64_t *punBytesTotal )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

int ISteamApps::GetAppBuildId()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

void ISteamApps::RequestAllProofOfPurchaseKeys()
{
    DEBUGLOGCALL(LCF_STEAM);
}

SteamAPICall_t ISteamApps::GetFileDetails( const char* pszFileName )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

}
