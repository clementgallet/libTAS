//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE_H_INCL

#include <stdint.h>
#include <string>
#include "../steamtypes.h"
#include "../../global.h"

namespace libtas {

void SteamSetRemoteStorageFolder(std::string path);

struct ISteamRemoteStorage
{
	union
	{
		const void *ptr;
		const struct ISteamRemoteStorage001Vtbl *v001;
		const struct ISteamRemoteStorage012Vtbl *v012;
		const struct ISteamRemoteStorage013Vtbl *v013;
		const struct ISteamRemoteStorage014Vtbl *v014;
	} vtbl;
};

ISteamRemoteStorage *SteamRemoteStorage_generic(const char *version);
void SteamRemoteStorage_set_version(const char *version);
OVERRIDE ISteamRemoteStorage *SteamRemoteStorage(void);

}

#endif
