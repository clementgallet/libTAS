//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage013.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage013Vtbl ISteamRemoteStorage013_vtbl = {
	ISteamRemoteStorage_FileWrite,
	ISteamRemoteStorage_FileRead,
	ISteamRemoteStorage_FileWriteAsync,
	ISteamRemoteStorage_FileReadAsync,
	ISteamRemoteStorage_FileReadAsyncComplete,
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
};

struct ISteamRemoteStorage *SteamRemoteStorage013(void)
{
	static struct ISteamRemoteStorage impl;

	impl.vtbl.v013 = &ISteamRemoteStorage013_vtbl;

	return &impl;
}

}
