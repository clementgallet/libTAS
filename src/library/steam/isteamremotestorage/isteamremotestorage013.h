//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMREMOTESTORAGE013_H_INCL
#define LIBTAS_ISTEAMREMOTESTORAGE013_H_INCL

#include <stdint.h>
// #include <string>
// #include "steamtypes.h"

#include "isteamremotestorage.h"
#define STEAMREMOTESTORAGE_INTERFACE_VERSION_013 "STEAMREMOTESTORAGE_INTERFACE_VERSION013"

namespace libtas {

struct ISteamRemoteStorage013Vtbl
{
	bool (*FileWrite)( void* iface, const char *pchFile, const void *pvData, int cubData );
	int	(*FileRead)( void* iface, const char *pchFile, void *pvData, int cubDataToRead );
	SteamAPICall_t (*FileWriteAsync)( void* iface, const char *pchFile, const void *pvData, unsigned int cubData );
	SteamAPICall_t (*FileReadAsync)( void* iface, const char *pchFile, unsigned int nOffset, unsigned int cubToRead );
	bool (*FileReadAsyncComplete)( void* iface, SteamAPICall_t hReadCall, void *pvBuffer, unsigned int cubToRead );
	bool (*FileForget)( void* iface, const char *pchFile );
	bool (*FileDelete)( void* iface, const char *pchFile );
	SteamAPICall_t (*FileShare)( void* iface, const char *pchFile );
	bool (*SetSyncPlatforms)( void* iface, const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );
	UGCFileWriteStreamHandle_t (*FileWriteStreamOpen)( void* iface, const char *pchFile );
	bool (*FileWriteStreamWriteChunk)( void* iface, UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int cubData );
	bool (*FileWriteStreamClose)( void* iface, UGCFileWriteStreamHandle_t writeHandle );
	bool (*FileWriteStreamCancel)( void* iface, UGCFileWriteStreamHandle_t writeHandle );
	bool (*FileExists)( void* iface, const char *pchFile );
	bool (*FilePersisted)( void* iface, const char *pchFile );
	int	(*GetFileSize)( void* iface, const char *pchFile );
	int64_t (*GetFileTimestamp)( void* iface, const char *pchFile );
	ERemoteStoragePlatform (*GetSyncPlatforms)( void* iface, const char *pchFile );
	int (*GetFileCount)(void* iface);
	const char *(*GetFileNameAndSize)( void* iface, int iFile, int *pnFileSizeInBytes );
	bool (*GetQuota)( void* iface, uint64_t *pnTotalBytes, uint64_t *puAvailableBytes );
	bool (*IsCloudEnabledForAccount)(void* iface);
	bool (*IsCloudEnabledForApp)(void* iface);
	void (*SetCloudEnabledForApp)( void* iface, bool bEnabled );
};

struct ISteamRemoteStorage *SteamRemoteStorage013(void);

}

#endif
