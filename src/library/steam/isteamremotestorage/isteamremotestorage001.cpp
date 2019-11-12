//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage001.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage001Vtbl ISteamRemoteStorage001_vtbl = {
	ISteamRemoteStorage_FileWrite,
	ISteamRemoteStorage_GetFileSize,
	ISteamRemoteStorage_FileRead,
	ISteamRemoteStorage_FileExists,
	ISteamRemoteStorage_FileDelete,
	ISteamRemoteStorage_GetFileCount,
	ISteamRemoteStorage_GetFileNameAndSize,
	ISteamRemoteStorage_GetQuota
};

struct ISteamRemoteStorage *SteamRemoteStorage001(void)
{
	static struct ISteamRemoteStorage impl;

	impl.vtbl.v001 = &ISteamRemoteStorage001_vtbl;

	return &impl;
}

}
