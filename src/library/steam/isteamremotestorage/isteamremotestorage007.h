//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE007_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE007_H_INCL

#include "isteamremotestorage.h"

#include <stdint.h>
#define STEAMREMOTESTORAGE_INTERFACE_VERSION_006 "STEAMREMOTESTORAGE_INTERFACE_VERSION006"
#define STEAMREMOTESTORAGE_INTERFACE_VERSION_007 "STEAMREMOTESTORAGE_INTERFACE_VERSION007"

namespace libtas {

struct ISteamRemoteStorage007Vtbl
{
    bool (*FileWrite)( void* iface, const char *pchFile, const void *pvData, int cubData );
    int (*FileRead)( void* iface, const char *pchFile, void *pvData, int cubDataToRead );
    bool (*FileForget)( void* iface, const char *pchFile );
    bool (*FileDelete)( void* iface, const char *pchFile );
    SteamAPICall_t (*FileShare)( void* iface, const char *pchFile );
    bool (*SetSyncPlatforms)( void* iface, const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );
    bool (*FileExists)( void* iface, const char *pchFile );
    bool (*FilePersisted)( void* iface, const char *pchFile );
    int (*GetFileSize)( void* iface, const char *pchFile );
    int64_t (*GetFileTimestamp)( void* iface, const char *pchFile );
    ERemoteStoragePlatform (*GetSyncPlatforms)( void* iface, const char *pchFile );
    int (*GetFileCount)(void* iface);
    const char *(*GetFileNameAndSize)( void* iface, int iFile, int *pnFileSizeInBytes );
    bool (*GetQuota)( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
    bool (*IsCloudEnabledForAccount)(void* iface);
    bool (*IsCloudEnabledForApp)(void* iface);
    void (*SetCloudEnabledForApp)( void* iface, bool bEnabled );
    SteamAPICall_t (*UGCDownload)( void* iface, UGCHandle_t hContent, unsigned int unPriority );
    bool (*GetUGCDownloadProgress)( void* iface, UGCHandle_t hContent, int *pnBytesDownloaded, int *pnBytesExpected );
    bool (*GetUGCDetails)( void* iface, UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int *pnFileSizeInBytes, CSteamID *pSteamIDOwner );
    int (*UGCRead)( void* iface, UGCHandle_t hContent, void *pvData, int cubDataToRead, unsigned int cOffset, EUGCReadAction eAction );
    int (*GetCachedUGCCount)(void* iface);
    UGCHandle_t (*GetCachedUGCHandle)( void* iface, int iCachedContent );
    SteamAPICall_t (*PublishWorkshopFile)( void* iface, const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType );
    PublishedFileUpdateHandle_t (*CreatePublishedFileUpdateRequest)( void* iface, PublishedFileId_t unPublishedFileId );
    bool (*UpdatePublishedFileFile)( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchFile );
    bool (*UpdatePublishedFilePreviewFile)( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile );
    bool (*UpdatePublishedFileTitle)( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchTitle );
    bool (*UpdatePublishedFileDescription)( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchDescription );
    bool (*UpdatePublishedFileVisibility)( void* iface, PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility );
    bool (*UpdatePublishedFileTags)( void* iface, PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags );
    SteamAPICall_t (*CommitPublishedFileUpdate)( void* iface, PublishedFileUpdateHandle_t updateHandle );
    SteamAPICall_t (*GetPublishedFileDetails)( void* iface, PublishedFileId_t unPublishedFileId, unsigned int unMaxSecondsOld );
    SteamAPICall_t (*DeletePublishedFile)( void* iface, PublishedFileId_t unPublishedFileId );
    SteamAPICall_t (*EnumerateUserPublishedFiles)( void* iface, unsigned int unStartIndex );
    SteamAPICall_t (*SubscribePublishedFile)( void* iface, PublishedFileId_t unPublishedFileId );
    SteamAPICall_t (*EnumerateUserSubscribedFiles)( void* iface, unsigned int unStartIndex );
    SteamAPICall_t (*UnsubscribePublishedFile)( void* iface, PublishedFileId_t unPublishedFileId );
    bool (*UpdatePublishedFileSetChangeDescription)( void* iface, PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription );
    SteamAPICall_t (*GetPublishedItemVoteDetails)( void* iface, PublishedFileId_t unPublishedFileId );
    SteamAPICall_t (*UpdateUserPublishedItemVote)( void* iface, PublishedFileId_t unPublishedFileId, bool bVoteUp );
    SteamAPICall_t (*GetUserPublishedItemVoteDetails)( void* iface, PublishedFileId_t unPublishedFileId );
    SteamAPICall_t (*EnumerateUserSharedWorkshopFiles)( void* iface, CSteamID steamId, unsigned int unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags );
    SteamAPICall_t (*PublishVideo)( void* iface, EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );
    SteamAPICall_t (*SetUserPublishedFileAction)( void* iface, PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction );
    SteamAPICall_t (*EnumeratePublishedFilesByUserAction)( void* iface, EWorkshopFileAction eAction, unsigned int unStartIndex );
    SteamAPICall_t (*EnumeratePublishedWorkshopFiles)( void* iface, EWorkshopEnumerationType eEnumerationType, unsigned int unStartIndex, unsigned int unCount, unsigned int unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags );
    SteamAPICall_t (*UGCDownloadToLocation)( void* iface, UGCHandle_t hContent, const char *pchLocation, unsigned int unPriority );
};

struct ISteamRemoteStorage *SteamRemoteStorage007(void);

}

#endif
