//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE003_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE003_H_INCL

#include "isteamremotestorage.h"

#include <stdint.h>
#define STEAMREMOTESTORAGE_INTERFACE_VERSION_003 "STEAMREMOTESTORAGE_INTERFACE_VERSION003"

namespace libtas {

struct ISteamRemoteStorage003Vtbl
{
    bool (*FileWrite)( void* iface, const char *pchFile, const void *pvData, int cubData );
    int (*FileRead)( void* iface, const char *pchFile, void *pvData, int cubDataToRead );
    bool (*FileForget)( void* iface, const char *pchFile );
    bool (*FileDelete)( void* iface, const char *pchFile );
    SteamAPICall_t (*FileShare)( void* iface, const char *pchFile );
    bool (*FileExists)( void* iface, const char *pchFile );
    bool (*FilePersisted)( void* iface, const char *pchFile );
    int (*GetFileSize)( void* iface, const char *pchFile );
    int64_t (*GetFileTimestamp)( void* iface, const char *pchFile );
    int (*GetFileCount)(void* iface);
    const char *(*GetFileNameAndSize)( void* iface, int iFile, int *pnFileSizeInBytes );
    bool (*GetQuota)( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
    bool (*IsCloudEnabledForAccount)(void* iface);
    bool (*IsCloudEnabledForApp)(void* iface);
    void (*SetCloudEnabledForApp)( void* iface, bool bEnabled );
    SteamAPICall_t (*UGCDownload)( void* iface, UGCHandle_t hContent, unsigned int unPriority );
    bool (*GetUGCDetails)( void* iface, UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int *pnFileSizeInBytes, CSteamID *pSteamIDOwner );
    int (*UGCRead)( void* iface, UGCHandle_t hContent, void *pvData, int cubDataToRead, unsigned int cOffset, EUGCReadAction eAction );
    int (*GetCachedUGCCount)(void* iface);
    UGCHandle_t (*GetCachedUGCHandle)( void* iface, int iCachedContent );
};

struct ISteamRemoteStorage *SteamRemoteStorage003(void);

}

#endif
