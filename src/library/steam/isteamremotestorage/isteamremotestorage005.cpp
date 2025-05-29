//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage005.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage005Vtbl ISteamRemoteStorage005_vtbl = {
    ISteamRemoteStorage_FileWrite,
    ISteamRemoteStorage_FileRead,
    ISteamRemoteStorage_FileForget,
    ISteamRemoteStorage_FileDelete,
    ISteamRemoteStorage_FileShare,
    ISteamRemoteStorage_SetSyncPlatforms,
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
    ISteamRemoteStorage_GetUGCDetails,
    ISteamRemoteStorage_UGCRead,
    ISteamRemoteStorage_GetCachedUGCCount,
    ISteamRemoteStorage_GetCachedUGCHandle,
    ISteamRemoteStorage_PublishFileOld,
    ISteamRemoteStorage_PublishWorkshopFileOld,
    ISteamRemoteStorage_CommitPublishedFileUpdate,
    ISteamRemoteStorage_GetPublishedFileDetails,
    ISteamRemoteStorage_DeletePublishedFile,
    ISteamRemoteStorage_EnumerateUserPublishedFiles,
    ISteamRemoteStorage_SubscribePublishedFile,
    ISteamRemoteStorage_EnumerateUserSubscribedFiles,
    ISteamRemoteStorage_UnsubscribePublishedFile,
};

struct ISteamRemoteStorage *SteamRemoteStorage005(void)
{
    static struct ISteamRemoteStorage impl;

    impl.vtbl.v005 = &ISteamRemoteStorage005_vtbl;

    return &impl;
}

}
