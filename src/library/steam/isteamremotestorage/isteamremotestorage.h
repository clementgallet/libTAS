//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE_H_INCL

#include "steam/steamtypes.h"
#include "hook.h"

#include <stdint.h>
#include <string>

namespace libtas {

void SteamSetRemoteStorageFolder(std::string path);

struct RemoteStorageFileWriteAsyncComplete_t
{
    EResult m_eResult;
};

struct ISteamRemoteStorage
{
    union
    {
        const void *ptr;
        const struct ISteamRemoteStorage001Vtbl *v001;
        const struct ISteamRemoteStorage002Vtbl *v002;
        const struct ISteamRemoteStorage003Vtbl *v003;
        const struct ISteamRemoteStorage005Vtbl *v005;
        const struct ISteamRemoteStorage007Vtbl *v007;
        const struct ISteamRemoteStorage012Vtbl *v012;
        const struct ISteamRemoteStorage013Vtbl *v013;
        const struct ISteamRemoteStorage014Vtbl *v014;
        const struct ISteamRemoteStorage016Vtbl *v016;
    } vtbl;
};

ISteamRemoteStorage *SteamRemoteStorage_generic(const char *version);
void SteamRemoteStorage_set_version(const char *version);
OVERRIDE ISteamRemoteStorage *SteamRemoteStorage(void);

}

#endif
