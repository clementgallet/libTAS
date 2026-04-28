/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <cstring>
#include <mutex>
#include <unordered_set>

namespace libtas {

namespace {

std::mutex ugcMutex;
UGCQueryHandle_t nextQueryHandle = 1;
UGCUpdateHandle_t nextUpdateHandle = 1;
std::unordered_set<UGCQueryHandle_t> activeQueryHandles;
std::unordered_set<UGCUpdateHandle_t> activeUpdateHandles;

UGCQueryHandle_t allocateQueryHandle()
{
    std::lock_guard<std::mutex> lock(ugcMutex);
    UGCQueryHandle_t handle = nextQueryHandle++;
    activeQueryHandles.insert(handle);
    return handle;
}

UGCUpdateHandle_t allocateUpdateHandle()
{
    std::lock_guard<std::mutex> lock(ugcMutex);
    UGCUpdateHandle_t handle = nextUpdateHandle++;
    activeUpdateHandles.insert(handle);
    return handle;
}

bool hasQueryHandle(UGCQueryHandle_t handle)
{
    std::lock_guard<std::mutex> lock(ugcMutex);
    return handle != 0 && activeQueryHandles.find(handle) != activeQueryHandles.end();
}

bool hasUpdateHandle(UGCUpdateHandle_t handle)
{
    std::lock_guard<std::mutex> lock(ugcMutex);
    return handle != 0 && activeUpdateHandles.find(handle) != activeUpdateHandles.end();
}

}

UGCQueryHandle_t ISteamUGC::CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage )
{
    LOGTRACE(LCF_STEAM);
	return allocateQueryHandle();
}

UGCQueryHandle_t ISteamUGC::CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage )
{
    LOGTRACE(LCF_STEAM);
    return allocateQueryHandle();
}

UGCQueryHandle_t ISteamUGC::CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32_t unNumPublishedFileIDs )
{
    LOGTRACE(LCF_STEAM);
    if (!pvecPublishedFileID || unNumPublishedFileIDs == 0)
        return 0;

    return allocateQueryHandle();
}

SteamAPICall_t ISteamUGC::SendQueryUGCRequest( UGCQueryHandle_t handle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamUGC::GetQueryUGCResult( UGCQueryHandle_t handle, uint32_t index, SteamUGCDetails_t *pDetails )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

bool ISteamUGC::GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32_t index, char *pchURL, uint32_t cchURLSize )
{
    LOGTRACE(LCF_STEAM);
    if (pchURL && cchURLSize > 0)
        pchURL[0] = '\0';
    return false;
}

bool ISteamUGC::GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32_t index, char *pchMetadata, uint32_t cchMetadatasize )
{
    LOGTRACE(LCF_STEAM);
    if (pchMetadata && cchMetadatasize > 0)
        pchMetadata[0] = '\0';
    return false;
}

bool ISteamUGC::GetQueryUGCChildren( UGCQueryHandle_t handle, uint32_t index, PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries )
{
    LOGTRACE(LCF_STEAM);
    if (pvecPublishedFileID && cMaxEntries > 0)
        std::memset(pvecPublishedFileID, 0, sizeof(*pvecPublishedFileID) * cMaxEntries);
    return false;
}

bool ISteamUGC::GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32_t index, EItemStatistic eStatType, uint64_t *pStatValue )
{
    LOGTRACE(LCF_STEAM);
    if (pStatValue)
        *pStatValue = 0;
    return false;
}

uint32_t ISteamUGC::GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32_t index )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamUGC::GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32_t index, uint32_t previewIndex, char *pchURLOrVideoID, uint32_t cchURLSize, char *pchOriginalFileName, uint32_t cchOriginalFileNameSize, EItemPreviewType *pPreviewType )
{
    LOGTRACE(LCF_STEAM);
    if (pchURLOrVideoID && cchURLSize > 0)
        pchURLOrVideoID[0] = '\0';
    if (pchOriginalFileName && cchOriginalFileNameSize > 0)
        pchOriginalFileName[0] = '\0';
    if (pPreviewType)
        *pPreviewType = 0;
    return false;
}

uint32_t ISteamUGC::GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32_t index )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

bool ISteamUGC::GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32_t index, uint32_t keyValueTagIndex, char *pchKey, uint32_t cchKeySize, char *pchValue, uint32_t cchValueSize )
{
    LOGTRACE(LCF_STEAM);
    if (pchKey && cchKeySize > 0)
        pchKey[0] = '\0';
    if (pchValue && cchValueSize > 0)
        pchValue[0] = '\0';
    return false;
}

bool ISteamUGC::ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
    LOGTRACE(LCF_STEAM);
    std::lock_guard<std::mutex> lock(ugcMutex);
    return activeQueryHandles.erase(handle) > 0;
}

bool ISteamUGC::AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32_t unDays )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetAllowCachedResponse( UGCQueryHandle_t handle, uint32_t unMaxAgeSeconds )
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}


bool ISteamUGC::SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName)
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag)
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetSearchText( UGCQueryHandle_t handle, const char *pSearchText)
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::SetRankedByTrendDays( UGCQueryHandle_t handle, uint32_t unDays)
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

bool ISteamUGC::AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue)
{
    LOGTRACE(LCF_STEAM);
    return hasQueryHandle(handle);
}

SteamAPICall_t ISteamUGC::RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32_t unMaxAgeSeconds)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

UGCUpdateHandle_t ISteamUGC::StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return allocateUpdateHandle();
}

bool ISteamUGC::SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(updateHandle);
}

bool ISteamUGC::SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32_t index, const char *pszPreviewFile)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32_t index, const char *pszVideoID)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

bool ISteamUGC::RemoveItemPreview( UGCUpdateHandle_t handle, uint32_t index)
{
    LOGTRACE(LCF_STEAM);
    return hasUpdateHandle(handle);
}

SteamAPICall_t ISteamUGC::SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

EItemUpdateStatus ISteamUGC::GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64_t *punBytesProcessed, uint64_t* punBytesTotal)
{
    LOGTRACE(LCF_STEAM);
    if (punBytesProcessed)
        *punBytesProcessed = 0;
    if (punBytesTotal) {
        *punBytesTotal = 0;
    }
	return 0;
}

SteamAPICall_t ISteamUGC::SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::GetUserItemVote( PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::SubscribeItem( PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

SteamAPICall_t ISteamUGC::UnsubscribeItem( PublishedFileId_t nPublishedFileID)
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

uint32_t ISteamUGC::GetNumSubscribedItems()
{
    LOGTRACE(LCF_STEAM);
	return 0;
}

uint32_t ISteamUGC::GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries)
{
    LOGTRACE(LCF_STEAM);
    if (pvecPublishedFileID && cMaxEntries > 0) {
        std::memset(pvecPublishedFileID, 0, sizeof(*pvecPublishedFileID) * cMaxEntries);
    }
	return 0;
}

}
