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

#include "isteamapps.h"

#include "logging.h"

namespace libtas {

bool ISteamApps::BIsSubscribed()
{
    LOGTRACE(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsLowViolence()
{
    LOGTRACE(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsCybercafe()
{
    LOGTRACE(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsVACBanned()
{
    LOGTRACE(LCF_STEAM);
	return false;
}

const char *ISteamApps::GetCurrentGameLanguage()
{
    LOGTRACE(LCF_STEAM);
	return "english";
}

const char *ISteamApps::GetAvailableGameLanguages()
{
    LOGTRACE(LCF_STEAM);
	return "english";
}

bool ISteamApps::BIsSubscribedApp( AppId_t appID )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsDlcInstalled( AppId_t appID )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetEarliestPurchaseUnixTime( AppId_t nAppID )
{
    LOGTRACE(LCF_STEAM);
	return 0;
}

bool ISteamApps::BIsSubscribedFromFreeWeekend()
{
    LOGTRACE(LCF_STEAM);
	return false;
}

int ISteamApps::GetDLCCount()
{
    LOGTRACE(LCF_STEAM);
	return 0;
}

bool ISteamApps::BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize )
{
    LOGTRACE(LCF_STEAM);
	return false;
}

void ISteamApps::InstallDLC( AppId_t nAppID )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamApps::UninstallDLC( AppId_t nAppID )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamApps::RequestAppProofOfPurchaseKey( AppId_t nAppID )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamApps::GetCurrentBetaName( char *pchName, int cchNameBufferSize )
{
    LOGTRACE(LCF_STEAM);
    strncpy(pchName, "public", cchNameBufferSize);
	return true;
}

bool ISteamApps::MarkContentCorrupt( bool bMissingFilesOnly )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, unsigned int cMaxDepots )
{
    LOGTRACE(LCF_STEAM);
	return 0;
}

unsigned int ISteamApps::GetAppInstallDir( AppId_t appID, char *pchFolder, unsigned int cchFolderBufferSize )
{
    LOGTRACE(LCF_STEAM | LCF_TODO);
	return 0;
}

bool ISteamApps::BIsAppInstalled( AppId_t appID )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

CSteamID ISteamApps::GetAppOwner()
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

const char *ISteamApps::GetLaunchQueryParam( const char *pchKey )
{
    LOGTRACE(LCF_STEAM);
	return "";
}

bool ISteamApps::GetDlcDownloadProgress( AppId_t nAppID, uint64_t *punBytesDownloaded, uint64_t *punBytesTotal )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

int ISteamApps::GetAppBuildId()
{
    LOGTRACE(LCF_STEAM);
	return 0;
}

void ISteamApps::RequestAllProofOfPurchaseKeys()
{
    LOGTRACE(LCF_STEAM);
}

SteamAPICall_t ISteamApps::GetFileDetails( const char* pszFileName )
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

}
