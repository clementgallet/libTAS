//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE_H_INCL

#include <stdint.h>
#include <string>
#include "steamtypes.h"

namespace libtas {

void SteamSetRemoteStorageFolder(std::string path);

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing, reading and writing files stored remotely
//			and cached locally
//-----------------------------------------------------------------------------
class ISteamRemoteStorage
{
	public:
		// NOTE
		//
		// Filenames are case-insensitive, and will be converted to lowercase automatically.
		// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
		// iterate the files, the filename returned will be "foo.bar".
		//

		// file operations
		virtual bool FileWrite( const char *pchFile, const void *pvData, int cubData );
		virtual int	FileRead( const char *pchFile, void *pvData, int cubDataToRead );

		virtual SteamAPICall_t FileWriteAsync( const char *pchFile, const void *pvData, unsigned int cubData );

		virtual SteamAPICall_t FileReadAsync( const char *pchFile, unsigned int nOffset, unsigned int cubToRead );
		virtual bool FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, unsigned int cubToRead );

		virtual bool FileForget( const char *pchFile );
		virtual bool FileDelete( const char *pchFile );
		virtual SteamAPICall_t FileShare( const char *pchFile );
		virtual bool SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );

		// file operations that cause network IO
		virtual UGCFileWriteStreamHandle_t FileWriteStreamOpen( const char *pchFile );
		virtual bool FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int cubData );
		virtual bool FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle );
		virtual bool FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle );

		// file information
		virtual bool FileExists( const char *pchFile );
		virtual bool FilePersisted( const char *pchFile );
		virtual int	GetFileSize( const char *pchFile );
		virtual int64_t GetFileTimestamp( const char *pchFile );
		virtual ERemoteStoragePlatform GetSyncPlatforms( const char *pchFile );

		// iteration
		virtual int GetFileCount();
		virtual const char *GetFileNameAndSize( int iFile, int *pnFileSizeInBytes );

		// configuration management
		virtual bool GetQuota( uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
		virtual bool IsCloudEnabledForAccount();
		virtual bool IsCloudEnabledForApp();
		virtual void SetCloudEnabledForApp( bool bEnabled );

		// // user generated content
		//
		// // Downloads a UGC file.  A priority value of 0 will download the file immediately,
		// // otherwise it will wait to download the file until all downloads with a lower priority
		// // value are completed.  Downloads with equal priority will occur simultaneously.
		// virtual SteamAPICall_t UGCDownload( UGCHandle_t hContent, unsigned int unPriority );
		//
		// // Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
		// // or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
		// virtual bool	GetUGCDownloadProgress( UGCHandle_t hContent, int *pnBytesDownloaded, int *pnBytesExpected );
		//
		// // Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
		// virtual bool	GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, OUT_STRING() char **ppchName, int *pnFileSizeInBytes, OUT_STRUCT() CSteamID *pSteamIDOwner );
		//
		// // After download, gets the content of the file.
		// // Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
		// // Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
		// // enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
		// // unless UGCDownload is called again.
		// // For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
		// virtual int	UGCRead( UGCHandle_t hContent, void *pvData, int cubDataToRead, unsigned int cOffset, EUGCReadAction eAction );
		//
		// // Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
		// virtual int	GetCachedUGCCount();
		// virtual	UGCHandle_t GetCachedUGCHandle( int iCachedContent );
		//
		// // publishing UGC
		// virtual SteamAPICall_t	PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType );
		// virtual PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId );
		// virtual bool UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile );
		// virtual bool UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile );
		// virtual bool UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle );
		// virtual bool UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription );
		// virtual bool UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility );
		// virtual bool UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags );
		// CALL_RESULT( RemoteStorageUpdatePublishedFileResult_t )
		// virtual SteamAPICall_t	CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle );
		// // Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
		// // cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
		// // A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
		// CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
		// virtual SteamAPICall_t	GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, unsigned int unMaxSecondsOld );
		// CALL_RESULT( RemoteStorageDeletePublishedFileResult_t )
		// virtual SteamAPICall_t	DeletePublishedFile( PublishedFileId_t unPublishedFileId );
		// // enumerate the files that the current user published with this app
		// CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
		// virtual SteamAPICall_t	EnumerateUserPublishedFiles( unsigned int unStartIndex );
		// CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
		// virtual SteamAPICall_t	SubscribePublishedFile( PublishedFileId_t unPublishedFileId );
		// CALL_RESULT( RemoteStorageEnumerateUserSubscribedFilesResult_t )
		// virtual SteamAPICall_t	EnumerateUserSubscribedFiles( unsigned int unStartIndex );
		// CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
		// virtual SteamAPICall_t	UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId );
		// virtual bool UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription );
		// CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
		// virtual SteamAPICall_t	GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId );
		// CALL_RESULT( RemoteStorageUpdateUserPublishedItemVoteResult_t )
		// virtual SteamAPICall_t	UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp );
		// CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
		// virtual SteamAPICall_t	GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId );
		// CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
		// virtual SteamAPICall_t	EnumerateUserSharedWorkshopFiles( CSteamID steamId, unsigned int unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags );
		// CALL_RESULT( RemoteStoragePublishFileProgress_t )
		// virtual SteamAPICall_t	PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );
		// CALL_RESULT( RemoteStorageSetUserPublishedFileActionResult_t )
		// virtual SteamAPICall_t	SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction );
		// CALL_RESULT( RemoteStorageEnumeratePublishedFilesByUserActionResult_t )
		// virtual SteamAPICall_t	EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, unsigned int unStartIndex );
		// // this method enumerates the public view of workshop files
		// CALL_RESULT( RemoteStorageEnumerateWorkshopFilesResult_t )
		// virtual SteamAPICall_t	EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, unsigned int unStartIndex, unsigned int unCount, unsigned int unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags );
		//
		// CALL_RESULT( RemoteStorageDownloadUGCResult_t )
		// virtual SteamAPICall_t UGCDownloadToLocation( UGCHandle_t hContent, const char *pchLocation, unsigned int unPriority );
};

}

#endif
