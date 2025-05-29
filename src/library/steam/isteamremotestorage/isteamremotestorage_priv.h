//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE_PRIV_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE_PRIV_H_INCL

#include "steam/steamtypes.h"

#include <stdint.h>
#include <string>

namespace libtas {

bool ISteamRemoteStorage_FileWrite( void* iface, const char *pchFile, const void *pvData, int cubData );
int ISteamRemoteStorage_FileRead( void* iface, const char *pchFile, void *pvData, int cubDataToRead );

SteamAPICall_t ISteamRemoteStorage_FileWriteAsync( void* iface, const char *pchFile, const void *pvData, unsigned int cubData );

SteamAPICall_t ISteamRemoteStorage_FileReadAsync( void* iface, const char *pchFile, unsigned int nOffset, unsigned int cubToRead );
bool ISteamRemoteStorage_FileReadAsyncComplete( void* iface, SteamAPICall_t hReadCall, void *pvBuffer, unsigned int cubToRead );

bool ISteamRemoteStorage_FileForget( void* iface, const char *pchFile );
bool ISteamRemoteStorage_FileDelete( void* iface, const char *pchFile );
SteamAPICall_t ISteamRemoteStorage_FileShare( void* iface, const char *pchFile );
bool ISteamRemoteStorage_SetSyncPlatforms( void* iface, const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );

// file operations that cause network IO
UGCFileWriteStreamHandle_t ISteamRemoteStorage_FileWriteStreamOpen( void* iface, const char *pchFile );
bool ISteamRemoteStorage_FileWriteStreamWriteChunk( void* iface, UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int cubData );
bool ISteamRemoteStorage_FileWriteStreamClose( void* iface, UGCFileWriteStreamHandle_t writeHandle );
bool ISteamRemoteStorage_FileWriteStreamCancel( void* iface, UGCFileWriteStreamHandle_t writeHandle );

// file information
bool ISteamRemoteStorage_FileExists( void* iface, const char *pchFile );
bool ISteamRemoteStorage_FilePersisted( void* iface, const char *pchFile );
int ISteamRemoteStorage_GetFileSize( void* iface, const char *pchFile );
int64_t ISteamRemoteStorage_GetFileTimestamp( void* iface, const char *pchFile );
ERemoteStoragePlatform ISteamRemoteStorage_GetSyncPlatforms( void* iface, const char *pchFile );

// iteration
int ISteamRemoteStorage_GetFileCount(void* iface);
const char *ISteamRemoteStorage_GetFileNameAndSize( void* iface, int iFile, int *pnFileSizeInBytes );

// configuration management
bool ISteamRemoteStorage_GetQuota( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
bool ISteamRemoteStorage_IsCloudEnabledForAccount(void* iface);
bool ISteamRemoteStorage_IsCloudEnabledForApp(void* iface);
void ISteamRemoteStorage_SetCloudEnabledForApp( void* iface, bool bEnabled );

// user generated content

// Downloads a UGC file.  A priority value of 0 will download the file immediately,
// otherwise it will wait to download the file until all downloads with a lower priority
// value are completed.  Downloads with equal priority will occur simultaneously.
SteamAPICall_t ISteamRemoteStorage_UGCDownload( void* iface, UGCHandle_t hContent, unsigned int unPriority );

// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
// or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
bool ISteamRemoteStorage_GetUGCDownloadProgress( void* iface, UGCHandle_t hContent, int *pnBytesDownloaded, int *pnBytesExpected );

// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
bool ISteamRemoteStorage_GetUGCDetails( void* iface, UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int *pnFileSizeInBytes, CSteamID *pSteamIDOwner );

// After download, gets the content of the file.
// Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
// Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
// enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
// unless UGCDownload is called again.
// For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
int ISteamRemoteStorage_UGCRead( void* iface, UGCHandle_t hContent, void *pvData, int cubDataToRead, unsigned int cOffset, EUGCReadAction eAction );

// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
int ISteamRemoteStorage_GetCachedUGCCount(void* iface);
UGCHandle_t ISteamRemoteStorage_GetCachedUGCHandle( void* iface, int iCachedContent );

// publishing UGC
SteamAPICall_t ISteamRemoteStorage_PublishWorkshopFile( void* iface, const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType );
SteamAPICall_t ISteamRemoteStorage_PublishFileOld( void* iface, const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );
SteamAPICall_t ISteamRemoteStorage_PublishWorkshopFileOld( void* iface, const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags );
PublishedFileUpdateHandle_t ISteamRemoteStorage_CreatePublishedFileUpdateRequest( void* iface, PublishedFileId_t unPublishedFileId );
bool ISteamRemoteStorage_UpdatePublishedFileFile( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchFile );
bool ISteamRemoteStorage_UpdatePublishedFilePreviewFile( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile );
bool ISteamRemoteStorage_UpdatePublishedFileTitle( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchTitle );
bool ISteamRemoteStorage_UpdatePublishedFileDescription( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchDescription );
bool ISteamRemoteStorage_UpdatePublishedFileVisibility( void* iface, PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility );
bool ISteamRemoteStorage_UpdatePublishedFileTags( void* iface, PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags );
SteamAPICall_t ISteamRemoteStorage_CommitPublishedFileUpdate( void* iface, PublishedFileUpdateHandle_t updateHandle );
// Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
// cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
// A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
SteamAPICall_t ISteamRemoteStorage_GetPublishedFileDetails( void* iface, PublishedFileId_t unPublishedFileId, unsigned int unMaxSecondsOld );
SteamAPICall_t ISteamRemoteStorage_DeletePublishedFile( void* iface, PublishedFileId_t unPublishedFileId );
// enumerate the files that the current user published with this app
SteamAPICall_t ISteamRemoteStorage_EnumerateUserPublishedFiles( void* iface, unsigned int unStartIndex );
SteamAPICall_t ISteamRemoteStorage_SubscribePublishedFile( void* iface, PublishedFileId_t unPublishedFileId );
SteamAPICall_t ISteamRemoteStorage_EnumerateUserSubscribedFiles( void* iface, unsigned int unStartIndex );
SteamAPICall_t ISteamRemoteStorage_UnsubscribePublishedFile( void* iface, PublishedFileId_t unPublishedFileId );
bool ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription );
SteamAPICall_t ISteamRemoteStorage_GetPublishedItemVoteDetails( void* iface, PublishedFileId_t unPublishedFileId );
SteamAPICall_t ISteamRemoteStorage_UpdateUserPublishedItemVote( void* iface, PublishedFileId_t unPublishedFileId, bool bVoteUp );
SteamAPICall_t ISteamRemoteStorage_GetUserPublishedItemVoteDetails( void* iface, PublishedFileId_t unPublishedFileId );
SteamAPICall_t ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles( void* iface, CSteamID steamId, unsigned int unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags );
SteamAPICall_t ISteamRemoteStorage_PublishVideo( void* iface, EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );
SteamAPICall_t ISteamRemoteStorage_SetUserPublishedFileAction( void* iface, PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction );
SteamAPICall_t ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( void* iface, EWorkshopFileAction eAction, unsigned int unStartIndex );
// this method enumerates the public view of workshop files
SteamAPICall_t ISteamRemoteStorage_EnumeratePublishedWorkshopFiles( void* iface, EWorkshopEnumerationType eEnumerationType, unsigned int unStartIndex, unsigned int unCount, unsigned int unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags );

SteamAPICall_t ISteamRemoteStorage_UGCDownloadToLocation( void* iface, UGCHandle_t hContent, const char *pchLocation, unsigned int unPriority );
// Cloud dynamic state change notification
int32_t ISteamRemoteStorage_GetLocalFileChangeCount();
const char *ISteamRemoteStorage_GetLocalFileChange( int iFile, ERemoteStorageLocalFileChange *pEChangeType, ERemoteStorageFilePathType *pEFilePathType );
// Indicate to Steam the beginning / end of a set of local file
// operations - for example, writing a game save that requires updating two files.
bool ISteamRemoteStorage_BeginFileWriteBatch();
bool ISteamRemoteStorage_EndFileWriteBatch();
}

#endif
