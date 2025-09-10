//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMCLIENT_H_INCL
#define LIBTAS_ISTEAMCLIENT_H_INCL

#include "steam/steamtypes.h"
#include "hook.h"

#include <stdint.h>
#include <string>

namespace libtas {

struct ISteamClient
{
    union
    {
        const void *ptr;
        const struct ISteamClient006Vtbl *v006;
        const struct ISteamClient012Vtbl *v012;
        const struct ISteamClient014Vtbl *v014;
        const struct ISteamClient016Vtbl *v016;
        const struct ISteamClient017Vtbl *v017;
        const struct ISteamClient020Vtbl *v020;
        const struct ISteamClient021Vtbl *v021;
    } vtbl;
};

ISteamClient *SteamClient_generic(const char *version);
void SteamClient_set_version(const char *version);
OVERRIDE ISteamClient *SteamClient(void);

}

#endif
