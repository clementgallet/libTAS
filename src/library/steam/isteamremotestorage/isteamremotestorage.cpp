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

#include "isteamremotestorage.h"
#include "isteamremotestorage_priv.h"
#include "isteamremotestorage001.h"
#include "isteamremotestorage012.h"
#include "isteamremotestorage013.h"
#include "isteamremotestorage014.h"
#include "../../logging.h"
#include "../../Utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h> 

namespace libtas {

char steamremotestorage[2048] = "/NOTVALID";
static const char *steamremotestorage_version = NULL;

void SteamSetRemoteStorageFolder(std::string path)
{
    DEBUGLOGCALL(LCF_STEAM);
    strncpy(steamremotestorage, path.c_str(), sizeof(steamremotestorage)-1);
}

struct ISteamRemoteStorage *SteamRemoteStorage_generic(const char *version)
{
	static const struct
	{
		const char *name;
		struct ISteamRemoteStorage *(*iface_getter)(void);
	} ifaces[] = {
		{ STEAMREMOTESTORAGE_INTERFACE_VERSION_001, SteamRemoteStorage001 },
		{ STEAMREMOTESTORAGE_INTERFACE_VERSION_012, SteamRemoteStorage012 },
		{ STEAMREMOTESTORAGE_INTERFACE_VERSION_013, SteamRemoteStorage013 },
		{ STEAMREMOTESTORAGE_INTERFACE_VERSION_014, SteamRemoteStorage014 },
		{ NULL, NULL }
	};
	int i;

    debuglogstdio(LCF_STEAM, "%s called with version %s", __func__, version);

	i = 0;
	while (ifaces[i].name)
	{
		if (strcmp(ifaces[i].name, version) == 0)
		{
			if (ifaces[i].iface_getter)
				return ifaces[i].iface_getter();

			break;
		}
		i++;
	}

    debuglogstdio(LCF_STEAM | LCF_WARNING, "Unable to find ISteamRemoteStorage version %s", version);

	return nullptr;
}

void SteamRemoteStorage_set_version(const char *version)
{
    debuglogstdio(LCF_STEAM, "%s called with version %s", __func__, version);

	if (!steamremotestorage_version)
		steamremotestorage_version = version;
}

struct ISteamRemoteStorage *SteamRemoteStorage(void)
{
	static struct ISteamRemoteStorage *cached_iface = nullptr;

    DEBUGLOGCALL(LCF_STEAM);

	if (!steamremotestorage_version)
	{
		steamremotestorage_version = STEAMREMOTESTORAGE_INTERFACE_VERSION_014;
        debuglogstdio(LCF_STEAM | LCF_WARNING, "ISteamRemoteStorage: No version specified, defaulting to %s", steamremotestorage_version);
	}

	if (!cached_iface)
		cached_iface = SteamRemoteStorage_generic(steamremotestorage_version);

	return cached_iface;
}

bool ISteamRemoteStorage_FileWrite( void* iface, const char *pchFile, const void *pvData, int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);

    /* Store the file locally */
    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;

    /* We don't use the native `open` function, so that we handle the
     * "prevent writing to disk" feature without extra work. */
    int fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd < 0)
        return false;

    Utils::writeAll(fd, pvData, cubData);

    if (close(fd) < 0) {
        return false;
    }

	return true;
}

int	ISteamRemoteStorage_FileRead( void* iface, const char *pchFile, void *pvData, int cubDataToRead )
{
    DEBUGLOGCALL(LCF_STEAM);

    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return 0;

    int ret = Utils::readAll(fd, pvData, cubDataToRead);

    if (close(fd) < 0) {
        return 0;
    }

	return ret;
}

SteamAPICall_t ISteamRemoteStorage_FileWriteAsync( void* iface, const char *pchFile, const void *pvData, unsigned int cubData )
{
    DEBUGLOGCALL(LCF_STEAM);
    /* Calling the sync version */
	ISteamRemoteStorage_FileWrite(iface, pchFile, pvData, cubData);
    return 1;
}

// static int asyncfd = 0;

SteamAPICall_t ISteamRemoteStorage_FileReadAsync( void* iface, const char *pchFile, unsigned int nOffset, unsigned int cubToRead )
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
    return 1;
}

bool ISteamRemoteStorage_FileReadAsyncComplete( void* iface, SteamAPICall_t hReadCall, void *pvBuffer, unsigned int cubToRead )
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
	return true;
}

bool ISteamRemoteStorage_FileForget( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_FileDelete( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);

    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;
    unlink(path.c_str());
	return true;
}

SteamAPICall_t ISteamRemoteStorage_FileShare( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage_SetSyncPlatforms( void* iface, const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

UGCFileWriteStreamHandle_t ISteamRemoteStorage_FileWriteStreamOpen( void* iface, const char *pchFile )
{
    debuglogstdio(LCF_STEAM, "%s called with file %s", __func__, pchFile);
    
    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;

    /* We don't use the native `open` function, so that we handle the
     * "prevent writing to disk" feature without extra work. */
    int fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    
	return fd;
}

bool ISteamRemoteStorage_FileWriteStreamWriteChunk( void* iface, UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int cubData )
{
    debuglogstdio(LCF_STEAM, "%s called with file handke %ull and size %d", __func__, writeHandle, cubData);
    
    ssize_t ret = write(writeHandle, pvData, cubData);
    
	return ret == cubData;
}

bool ISteamRemoteStorage_FileWriteStreamClose( void* iface, UGCFileWriteStreamHandle_t writeHandle )
{
    debuglogstdio(LCF_STEAM, "%s called with file handke %ull", __func__, writeHandle);

    int ret = close(writeHandle);

	return ret == 0;
}

bool ISteamRemoteStorage_FileWriteStreamCancel( void* iface, UGCFileWriteStreamHandle_t writeHandle )
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);

    /* TODO: Not good, should not write or overwrite file */
    int ret = close(writeHandle);

	return ret == 0;
}

bool ISteamRemoteStorage_FileExists( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;
    return (access(path.c_str(), F_OK) == 0);
}

bool ISteamRemoteStorage_FilePersisted( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

int	ISteamRemoteStorage_GetFileSize( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
    std::string path = steamremotestorage;
    path += "/";
    path += pchFile;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return 0;

    int ret = lseek(fd, 0, SEEK_END);

    close(fd);

    if (ret < 0) {
        return 0;
    }

	return ret;
}

int64_t ISteamRemoteStorage_GetFileTimestamp( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM | LCF_TODO);
	return 0;
}

ERemoteStoragePlatform ISteamRemoteStorage_GetSyncPlatforms( void* iface, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

int ISteamRemoteStorage_GetFileCount(void* iface)
{
    DEBUGLOGCALL(LCF_STEAM);
    
    std::string path = steamremotestorage;
    path += "/";
    
    DIR *d = opendir(path.c_str());
    
    if (!d)
        return 0;

    int filecount = 0;
    struct dirent *dir;
    while ((dir = readdir(d)) != nullptr) {
        /* Skip the special files `..` and `.` */
        if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
            filecount++;
    }
    
    closedir(d);

    debuglogstdio(LCF_STEAM, "   return file count %d", filecount);
	return filecount;
}

const char *ISteamRemoteStorage_GetFileNameAndSize( void* iface, int iFile, int *pnFileSizeInBytes )
{
    DEBUGLOGCALL(LCF_STEAM);

    std::string path = steamremotestorage;
    path += "/";
    
    DIR *d = opendir(path.c_str());
    
    if (!d)
        return 0;

    struct dirent *dir;
    int i = 0;

    do {
        dir = readdir(d);

        /* Skip the special files `..` and `.` */
        while (dir != nullptr && strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
            dir = readdir(d);

        i++;
    } while (dir != nullptr && i <= iFile);

    closedir(d);

    if (dir == nullptr) {
        *pnFileSizeInBytes = 0;
        return "";
    }

    *pnFileSizeInBytes = ISteamRemoteStorage_GetFileSize(iface, dir->d_name);
    debuglogstdio(LCF_STEAM, "   return file %s and size %d", dir->d_name, *pnFileSizeInBytes);

    return dir->d_name;
}

bool ISteamRemoteStorage_GetQuota( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_IsCloudEnabledForAccount(void* iface)
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamRemoteStorage_IsCloudEnabledForApp(void* iface)
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

void ISteamRemoteStorage_SetCloudEnabledForApp( void* iface, bool bEnabled )
{
    DEBUGLOGCALL(LCF_STEAM);
}

SteamAPICall_t ISteamRemoteStorage_UGCDownload( void* iface, UGCHandle_t hContent, unsigned int unPriority )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
// or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
bool ISteamRemoteStorage_GetUGCDownloadProgress( void* iface, UGCHandle_t hContent, int *pnBytesDownloaded, int *pnBytesExpected )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
bool ISteamRemoteStorage_GetUGCDetails( void* iface, UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int *pnFileSizeInBytes, CSteamID *pSteamIDOwner )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

// After download, gets the content of the file.
// Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
// Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
// enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
// unless UGCDownload is called again.
// For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
int	ISteamRemoteStorage_UGCRead( void* iface, UGCHandle_t hContent, void *pvData, int cubDataToRead, unsigned int cOffset, EUGCReadAction eAction )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
int	ISteamRemoteStorage_GetCachedUGCCount(void* iface)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

UGCHandle_t ISteamRemoteStorage_GetCachedUGCHandle( void* iface, int iCachedContent )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// publishing UGC
SteamAPICall_t	ISteamRemoteStorage_PublishWorkshopFile( void* iface, const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

PublishedFileUpdateHandle_t ISteamRemoteStorage_CreatePublishedFileUpdateRequest( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage_UpdatePublishedFileFile( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_UpdatePublishedFilePreviewFile( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_UpdatePublishedFileTitle( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchTitle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_UpdatePublishedFileDescription( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchDescription )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_UpdatePublishedFileVisibility( void* iface, PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamRemoteStorage_UpdatePublishedFileTags( void* iface, PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

SteamAPICall_t	ISteamRemoteStorage_CommitPublishedFileUpdate( void* iface, PublishedFileUpdateHandle_t updateHandle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
// cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
// A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
SteamAPICall_t	ISteamRemoteStorage_GetPublishedFileDetails( void* iface, PublishedFileId_t unPublishedFileId, unsigned int unMaxSecondsOld )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_DeletePublishedFile( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// enumerate the files that the current user published with this app
SteamAPICall_t	ISteamRemoteStorage_EnumerateUserPublishedFiles( void* iface, unsigned int unStartIndex )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_SubscribePublishedFile( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_EnumerateUserSubscribedFiles( void* iface, unsigned int unStartIndex )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_UnsubscribePublishedFile( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

SteamAPICall_t	ISteamRemoteStorage_GetPublishedItemVoteDetails( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_UpdateUserPublishedItemVote( void* iface, PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_GetUserPublishedItemVoteDetails( void* iface, PublishedFileId_t unPublishedFileId )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles( void* iface, CSteamID steamId, unsigned int unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_PublishVideo( void* iface, EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_SetUserPublishedFileAction( void* iface, PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t	ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( void* iface, EWorkshopFileAction eAction, unsigned int unStartIndex )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

// this method enumerates the public view of workshop files
SteamAPICall_t	ISteamRemoteStorage_EnumeratePublishedWorkshopFiles( void* iface, EWorkshopEnumerationType eEnumerationType, unsigned int unStartIndex, unsigned int unCount, unsigned int unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamRemoteStorage_UGCDownloadToLocation( void* iface, UGCHandle_t hContent, const char *pchLocation, unsigned int unPriority )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

}
