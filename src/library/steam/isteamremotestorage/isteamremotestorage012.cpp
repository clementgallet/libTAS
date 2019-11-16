//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage012.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage012Vtbl ISteamRemoteStorage012_vtbl = {
	ISteamRemoteStorage_FileWrite,
	ISteamRemoteStorage_FileRead,
	ISteamRemoteStorage_FileForget,
	ISteamRemoteStorage_FileDelete,
	ISteamRemoteStorage_FileShare,
	ISteamRemoteStorage_SetSyncPlatforms,
	ISteamRemoteStorage_FileWriteStreamOpen,
	ISteamRemoteStorage_FileWriteStreamWriteChunk,
	ISteamRemoteStorage_FileWriteStreamClose,
	ISteamRemoteStorage_FileWriteStreamCancel,
	ISteamRemoteStorage_FileExists,
	ISteamRemoteStorage_FilePersisted,
	ISteamRemoteStorage_GetFileSize,
	ISteamRemoteStorage_GetFileTimestamp,
	ISteamRemoteStorage_GetSyncPlatforms,
	ISteamRemoteStorage_GetFileCount,
	ISteamRemoteStorage_GetFileNameAndSize,
	ISteamRemoteStorage_GetQuota,
	ISteamRemoteStorage_IsCloudEnabledForAccount,
	ISteamRemoteStorage_IsCloudEnabledForApp,
	ISteamRemoteStorage_SetCloudEnabledForApp,
	ISteamRemoteStorage_UGCDownload,
	ISteamRemoteStorage_GetUGCDownloadProgress,
	ISteamRemoteStorage_GetUGCDetails,
	ISteamRemoteStorage_UGCRead,
	ISteamRemoteStorage_GetCachedUGCCount,
	ISteamRemoteStorage_GetCachedUGCHandle,
	ISteamRemoteStorage_PublishWorkshopFile,
	ISteamRemoteStorage_CreatePublishedFileUpdateRequest,
	ISteamRemoteStorage_UpdatePublishedFileFile,
	ISteamRemoteStorage_UpdatePublishedFilePreviewFile,
	ISteamRemoteStorage_UpdatePublishedFileTitle,
	ISteamRemoteStorage_UpdatePublishedFileDescription,
	ISteamRemoteStorage_UpdatePublishedFileVisibility,
	ISteamRemoteStorage_UpdatePublishedFileTags,
	ISteamRemoteStorage_CommitPublishedFileUpdate,
	ISteamRemoteStorage_GetPublishedFileDetails,
	ISteamRemoteStorage_DeletePublishedFile,
	ISteamRemoteStorage_EnumerateUserPublishedFiles,
	ISteamRemoteStorage_SubscribePublishedFile,
	ISteamRemoteStorage_EnumerateUserSubscribedFiles,
	ISteamRemoteStorage_UnsubscribePublishedFile,
	ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription,
	ISteamRemoteStorage_GetPublishedItemVoteDetails,
	ISteamRemoteStorage_UpdateUserPublishedItemVote,
	ISteamRemoteStorage_GetUserPublishedItemVoteDetails,
	ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles,
	ISteamRemoteStorage_PublishVideo,
	ISteamRemoteStorage_SetUserPublishedFileAction,
	ISteamRemoteStorage_EnumeratePublishedFilesByUserAction,
	ISteamRemoteStorage_EnumeratePublishedWorkshopFiles,
	ISteamRemoteStorage_UGCDownloadToLocation,
};

struct ISteamRemoteStorage *SteamRemoteStorage012(void)
{
	static struct ISteamRemoteStorage impl;

	impl.vtbl.v012 = &ISteamRemoteStorage012_vtbl;

	return &impl;
}

}
