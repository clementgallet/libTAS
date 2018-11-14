//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_STEAMTYPES_H_INCL
#define LIBTAS_STEAMTYPES_H_INCL

#include <stdint.h>

namespace libtas {

typedef int HSteamUser;
typedef int HSteamPipe;
typedef unsigned int HAuthTicket;
typedef uint32_t ScreenshotHandle;

typedef uint64_t CSteamID;
typedef uint64_t CGameID;

typedef unsigned int AppId_t;
typedef uint64_t SteamAPICall_t;

typedef uint64_t PublishedFileId_t; // not sure
typedef uint64_t SteamParamStringArray_t; // not sure

typedef uint64_t UGCFileWriteStreamHandle_t;
typedef uint64_t UGCQueryHandle_t;
typedef uint64_t UGCUpdateHandle_t;
typedef uint32_t AccountID_t;
typedef uint32_t PartnerId_t;
typedef void SteamUGCDetails_t;
typedef void FriendGameInfo_t;
typedef void LeaderboardEntry_t;

typedef int EVoiceResult;
typedef int EBeginAuthSessionResult;
typedef int EUserHasLicenseForAppResult;
typedef int ELeaderboardSortMethod;
typedef int ELeaderboardDisplayType;
typedef int ELeaderboardDataRequest;
typedef int ESteamAPICallFailure;
typedef int EGamepadTextInputMode;
typedef int EGamepadTextInputLineMode;
typedef int EUniverse;
typedef int ENotificationPosition;
typedef int ERemoteStoragePlatform;
typedef int EAccountType;
typedef int EPersonaState;
typedef int EFriendRelationship;
typedef int EVRScreenshotType;

typedef int EUGCQuery;
typedef int EUGCMatchingUGCType;
typedef int EUserUGCList;
typedef int EUserUGCListSortOrder;
typedef int EItemStatistic;
typedef int EItemPreviewType;
typedef int EItemUpdateStatus;
typedef int ERemoteStoragePublishedFileVisibility;
typedef int EWorkshopFileType;
typedef int ELeaderboardUploadScoreMethod;



typedef void (*SteamAPIWarningMessageHook_t)(int, const char *);
typedef uint32_t ( *SteamAPI_CheckCallbackRegistered_t )( int iCallbackNum );



}

#endif
