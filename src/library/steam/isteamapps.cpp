/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <cstring>

namespace libtas {

bool ISteamApps::BIsSubscribed()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsLowViolence()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsCybercafe()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsVACBanned()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

const char *ISteamApps::GetCurrentGameLanguage()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return "english";
}

const char *ISteamApps::GetAvailableGameLanguages()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return "english";
}

bool ISteamApps::BIsSubscribedApp( AppId_t appID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsDlcInstalled( AppId_t appID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetEarliestPurchaseUnixTime( AppId_t nAppID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 0;
}

bool ISteamApps::BIsSubscribedFromFreeWeekend()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return false;
}

int ISteamApps::GetDLCCount()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 0;
}

bool ISteamApps::BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);

    if (pAppID)
        *pAppID = 0;
    if (pbAvailable)
        *pbAvailable = false;
    if (pchName && cchNameBufferSize > 0) {
        pchName[0] = '\0';
    }

	return false;
}

void ISteamApps::InstallDLC( AppId_t nAppID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
}

void ISteamApps::UninstallDLC( AppId_t nAppID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
}

void ISteamApps::RequestAppProofOfPurchaseKey( AppId_t nAppID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
}

bool ISteamApps::GetCurrentBetaName( char *pchName, int cchNameBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (pchName == nullptr || cchNameBufferSize <= 0)
        return false;

    strncpy(pchName, "public", cchNameBufferSize);
	 pchName[cchNameBufferSize-1] = '\0';
	return true;
}

bool ISteamApps::MarkContentCorrupt( bool bMissingFilesOnly )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return true;
}

unsigned int ISteamApps::GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, unsigned int cMaxDepots )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (pvecDepots && cMaxDepots > 0) {
        std::memset(pvecDepots, 0, sizeof(*pvecDepots) * cMaxDepots);
    }
	return 0;
}

unsigned int ISteamApps::GetAppInstallDir( AppId_t appID, char *pchFolder, unsigned int cchFolderBufferSize )
{
    LOGTRACE_SIMPLE(LCF_STEAM | LCF_TODO);
    if (pchFolder && cchFolderBufferSize > 0) {
        pchFolder[0] = '\0';
    }
	return 0;
}

bool ISteamApps::BIsAppInstalled( AppId_t appID )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return true;
}

CSteamID ISteamApps::GetAppOwner()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 1;
}

const char *ISteamApps::GetLaunchQueryParam( const char *pchKey )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return "";
}

bool ISteamApps::GetDlcDownloadProgress( AppId_t nAppID, uint64_t *punBytesDownloaded, uint64_t *punBytesTotal )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
    if (punBytesDownloaded)
        *punBytesDownloaded = 0;
    if (punBytesTotal) {
        *punBytesTotal = 0;
    }
	return true;
}

int ISteamApps::GetAppBuildId()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 0;
}

void ISteamApps::RequestAllProofOfPurchaseKeys()
{
    LOGTRACE_SIMPLE(LCF_STEAM);
}

SteamAPICall_t ISteamApps::GetFileDetails( const char* pszFileName )
{
    LOGTRACE_SIMPLE(LCF_STEAM);
	return 1;
}

}
