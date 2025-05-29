//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSERSTATS_H_INCL
#define LIBTAS_ISTEAMUSERSTATS_H_INCL

#include "steam/steamtypes.h"
#include "hook.h"

#include <stdint.h>
#include <string>

namespace libtas {

struct ISteamUserStats
{
    union
    {
        const void *ptr;
        const struct ISteamUserStats003Vtbl *v003;
        const struct ISteamUserStats004Vtbl *v004;
        const struct ISteamUserStats005Vtbl *v005;
        const struct ISteamUserStats006Vtbl *v006;
        const struct ISteamUserStats007Vtbl *v007;
        const struct ISteamUserStats008Vtbl *v008;
        const struct ISteamUserStats009Vtbl *v009;
        const struct ISteamUserStats010Vtbl *v010;
        const struct ISteamUserStats011Vtbl *v011;
        const struct ISteamUserStats012Vtbl *v012;
        const struct ISteamUserStats013Vtbl *v013;
    } vtbl;
};

ISteamUserStats *SteamUserStats_generic(const char *version);
void SteamUserStats_set_version(const char *version);
OVERRIDE ISteamUserStats *SteamUserStats(void);

}

#endif
