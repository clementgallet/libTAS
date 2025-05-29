//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#include "isteamremotestorage002.h"
#include "isteamremotestorage_priv.h"

namespace libtas {

static const struct ISteamRemoteStorage002Vtbl ISteamRemoteStorage002_vtbl = {
    ISteamRemoteStorage_FileWrite,
    ISteamRemoteStorage_GetFileSize,
    ISteamRemoteStorage_FileRead,
    ISteamRemoteStorage_FileExists,
    ISteamRemoteStorage_GetFileCount,
    ISteamRemoteStorage_GetFileNameAndSize,
    ISteamRemoteStorage_GetQuota
};

struct ISteamRemoteStorage *SteamRemoteStorage002(void)
{
    static struct ISteamRemoteStorage impl;

    impl.vtbl.v002 = &ISteamRemoteStorage002_vtbl;

    return &impl;
}

}
