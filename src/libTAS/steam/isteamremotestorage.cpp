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

#include "isteamremotestorage.h"
#include "../logging.h"

namespace libtas {

bool ISteamRemoteStorage::FileWrite( const char *pchFile, const void *pvData, int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

int	ISteamRemoteStorage::FileRead( const char *pchFile, void *pvData, int cubDataToRead )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

SteamAPICall_t ISteamRemoteStorage::FileWriteAsync( const char *pchFile, const void *pvData, unsigned int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamRemoteStorage::FileReadAsync( const char *pchFile, unsigned int nOffset, unsigned int cubToRead )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage::FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, unsigned int cubToRead )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::FileForget( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::FileDelete( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

SteamAPICall_t ISteamRemoteStorage::FileShare( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage::SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

UGCFileWriteStreamHandle_t ISteamRemoteStorage::FileWriteStreamOpen( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage::FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::FileExists( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamRemoteStorage::FilePersisted( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

int	ISteamRemoteStorage::GetFileSize( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

int64_t ISteamRemoteStorage::GetFileTimestamp( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

ERemoteStoragePlatform ISteamRemoteStorage::GetSyncPlatforms( const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

int ISteamRemoteStorage::GetFileCount()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

const char *ISteamRemoteStorage::GetFileNameAndSize( int iFile, int *pnFileSizeInBytes )
{
    DEBUGLOGCALL(LCF_STEAM);
	return "";
}

bool ISteamRemoteStorage::GetQuota( uint64_t *pnTotalBytes, uint64_t *puAvailableBytes )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage::IsCloudEnabledForAccount()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamRemoteStorage::IsCloudEnabledForApp()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

void ISteamRemoteStorage::SetCloudEnabledForApp( bool bEnabled )
{
    DEBUGLOGCALL(LCF_STEAM);
}

}
