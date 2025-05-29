//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE002_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE002_H_INCL

#include "isteamremotestorage.h"

#include <stdint.h>
#define STEAMREMOTESTORAGE_INTERFACE_VERSION_002 "STEAMREMOTESTORAGE_INTERFACE_VERSION002"

namespace libtas {

struct ISteamRemoteStorage002Vtbl
{
    bool (*FileWrite)( void* iface, const char *pchFile, const void *pvData, int cubData );
    int (*GetFileSize)( void* iface, const char *pchFile );
    int (*FileRead)( void* iface, const char *pchFile, void *pvData, int cubDataToRead );
    bool (*FileExists)( void* iface, const char *pchFile );
    int (*GetFileCount)(void* iface);
    const char *(*GetFileNameAndSize)( void* iface, int iFile, int *pnFileSizeInBytes );
    bool (*GetQuota)( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
};

struct ISteamRemoteStorage *SteamRemoteStorage002(void);

}

#endif
