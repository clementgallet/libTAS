//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSER_H_INCL
#define LIBTAS_ISTEAMUSER_H_INCL

#include "steam/steamtypes.h"
#include "hook.h"

#include <stdint.h>
#include <string>

namespace libtas {

void SteamSetUserDataFolder(std::string path);

struct ISteamUser
{
	union
	{
		const void *ptr;
        const struct ISteamUser021Vtbl *v021;
        const struct ISteamUser023Vtbl *v023;
	} vtbl;
};

ISteamUser *SteamUser_generic(const char *version);
void SteamUser_set_version(const char *version);
OVERRIDE ISteamUser *SteamUser(void);

}

#endif
