//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage003.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage003Vtbl ISteamRemoteStorage003_vtbl = {
    ISteamRemoteStorage_FileWrite,
    ISteamRemoteStorage_FileRead,
    ISteamRemoteStorage_FileForget,
    ISteamRemoteStorage_FileDelete,
    ISteamRemoteStorage_FileShare,
    ISteamRemoteStorage_FileExists,
    ISteamRemoteStorage_FilePersisted,
    ISteamRemoteStorage_GetFileSize,
    ISteamRemoteStorage_GetFileTimestamp,
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
};

struct ISteamRemoteStorage *SteamRemoteStorage003(void)
{
    static struct ISteamRemoteStorage impl;

    impl.vtbl.v003 = &ISteamRemoteStorage003_vtbl;

    return &impl;
}

}
