/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "isteamugc.h"

#include "logging.h"

namespace libtas {

UGCQueryHandle_t ISteamUGC::CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

UGCQueryHandle_t ISteamUGC::CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

UGCQueryHandle_t ISteamUGC::CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32_t unNumPublishedFileIDs )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::SendQueryUGCRequest( UGCQueryHandle_t handle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamUGC::GetQueryUGCResult( UGCQueryHandle_t handle, uint32_t index, SteamUGCDetails_t *pDetails )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32_t index, char *pchURL, uint32_t cchURLSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32_t index, char *pchMetadata, uint32_t cchMetadatasize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::GetQueryUGCChildren( UGCQueryHandle_t handle, uint32_t index, PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32_t index, EItemStatistic eStatType, uint64_t *pStatValue )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

uint32_t ISteamUGC::GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32_t index )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamUGC::GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32_t index, uint32_t previewIndex, char *pchURLOrVideoID, uint32_t cchURLSize, char *pchOriginalFileName, uint32_t cchOriginalFileNameSize, EItemPreviewType *pPreviewType )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

uint32_t ISteamUGC::GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32_t index )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamUGC::GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32_t index, uint32_t keyValueTagIndex, char *pchKey, uint32_t cchKeySize, char *pchValue, uint32_t cchValueSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32_t unDays )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetAllowCachedResponse( UGCQueryHandle_t handle, uint32_t unMaxAgeSeconds )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}


bool ISteamUGC::SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetSearchText( UGCQueryHandle_t handle, const char *pSearchText)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetRankedByTrendDays( UGCQueryHandle_t handle, uint32_t unDays)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

SteamAPICall_t ISteamUGC::RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32_t unMaxAgeSeconds)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

UGCUpdateHandle_t ISteamUGC::StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

bool ISteamUGC::SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32_t index, const char *pszPreviewFile)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32_t index, const char *pszVideoID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUGC::RemoveItemPreview( UGCUpdateHandle_t handle, uint32_t index)
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

SteamAPICall_t ISteamUGC::SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

EItemUpdateStatus ISteamUGC::GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64_t *punBytesProcessed, uint64_t* punBytesTotal)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

SteamAPICall_t ISteamUGC::SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::GetUserItemVote( PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::SubscribeItem( PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

SteamAPICall_t ISteamUGC::UnsubscribeItem( PublishedFileId_t nPublishedFileID)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

uint32_t ISteamUGC::GetNumSubscribedItems()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

uint32_t ISteamUGC::GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries)
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

}
