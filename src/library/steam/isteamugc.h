//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUGC_H_INCL
#define LIBTAS_ISTEAMUGC_H_INCL

#include "steamtypes.h"

#include <stdint.h>

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: Steam UGC support API
//-----------------------------------------------------------------------------
class ISteamUGC
{
public:

	// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage );

	// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32_t unPage );

	// Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
	virtual UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32_t unNumPublishedFileIDs );

	// Send the query to Steam
	virtual SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle );

	// Retrieve an individual result after receiving the callback for querying UGC
	virtual bool GetQueryUGCResult( UGCQueryHandle_t handle, uint32_t index, SteamUGCDetails_t *pDetails );
	virtual bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32_t index, char *pchURL, uint32_t cchURLSize );
	virtual bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32_t index, char *pchMetadata, uint32_t cchMetadatasize );
	virtual bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32_t index, PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries );
	virtual bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32_t index, EItemStatistic eStatType, uint64_t *pStatValue );
	virtual uint32_t GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32_t index );
	virtual bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32_t index, uint32_t previewIndex, char *pchURLOrVideoID, uint32_t cchURLSize, char *pchOriginalFileName, uint32_t cchOriginalFileNameSize, EItemPreviewType *pPreviewType );
	virtual uint32_t GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32_t index );
	virtual bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32_t index, uint32_t keyValueTagIndex, char *pchKey, uint32_t cchKeySize, char *pchValue, uint32_t cchValueSize );

	// Release the request to free up memory, after retrieving results
	virtual bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle );

	// Options to set for querying UGC
	virtual bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName );
	virtual bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName );
	virtual bool SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs );
	virtual bool SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags );
	virtual bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription );
	virtual bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata );
	virtual bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren );
	virtual bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews );
	virtual bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly );
	virtual bool SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32_t unDays );
	virtual bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage );
	virtual bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32_t unMaxAgeSeconds );

	// Options only for querying user UGC
	virtual bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName );

	// Options only for querying all UGC
	virtual bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag );
	virtual bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText );
	virtual bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32_t unDays );
	virtual bool AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue );

	// DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
	virtual SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32_t unMaxAgeSeconds );

	// Steam Workshop Creator API
	virtual SteamAPICall_t CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType ); // create new item for this app with no content attached yet

	virtual UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID ); // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()

	virtual bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle ); // change the title of an UGC item
	virtual bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription ); // change the description of an UGC item
	virtual bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage ); // specify the language of the title or description that will be set
	virtual bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData ); // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)
	virtual bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility ); // change the visibility of an UGC item
	virtual bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags ); // change the tags of an UGC item
	virtual bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder ); // update item content from this local folder
	virtual bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile ); //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size
	virtual bool RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey ); // remove any existing key-value tags with the specified key
	virtual bool AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue ); // add new key-value tags for the item. Note that there can be multiple values for a tag.
	virtual bool AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type ); //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size
	virtual bool AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID ); //  add preview video for this item
	virtual bool UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32_t index, const char *pszPreviewFile ); //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size
	virtual bool UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32_t index, const char *pszVideoID ); //  updates an existing preview video for this item
	virtual bool RemoveItemPreview( UGCUpdateHandle_t handle, uint32_t index ); // remove a preview by index starting at 0 (previews are sorted)

	virtual SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote ); // commit update process started with StartItemUpdate()
	virtual EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64_t *punBytesProcessed, uint64_t* punBytesTotal );

	// Steam Workshop Consumer API
	virtual SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp );
	virtual SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID );
	virtual SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID );
	virtual SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID );
	virtual SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID ); // subscribe to this item, will be installed ASAP
	virtual SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID ); // unsubscribe from this item, will be uninstalled after game quits
	virtual uint32_t GetNumSubscribedItems(); // number of subscribed items
	virtual uint32_t GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32_t cMaxEntries ); // all subscribed item PublishFileIDs

	// // get EItemState flags about item on this client
	// virtual uint32_t GetItemState( PublishedFileId_t nPublishedFileID );
	//
	// // get info about currently installed content on disc for items that have k_EItemStateInstalled set
	// // if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
	// virtual bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64_t *punSizeOnDisk, OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32_t cchFolderSize, uint32_t *punTimeStamp );
	//
	// // get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
	// virtual bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64_t *punBytesDownloaded, uint64_t *punBytesTotal );
	//
	// // download new or update already installed item. If function returns true, wait for DownloadItemResult_t. If the item is already installed,
	// // then files on disk should not be used until callback received. If item is not subscribed to, it will be cached for some time.
	// // If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
	// virtual bool DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority );
	//
	// // game servers can set a specific workshop folder before issuing any UGC commands.
	// // This is helpful if you want to support multiple game servers running out of the same install folder
	// virtual bool BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder );
	//
	// // SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
	// virtual void SuspendDownloads( bool bSuspend );
	//
	// // usage tracking
	// CALL_RESULT( StartPlaytimeTrackingResult_t )
	// virtual SteamAPICall_t StartPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32_t unNumPublishedFileIDs );
	// CALL_RESULT( StopPlaytimeTrackingResult_t )
	// virtual SteamAPICall_t StopPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32_t unNumPublishedFileIDs );
	// CALL_RESULT( StopPlaytimeTrackingResult_t )
	// virtual SteamAPICall_t StopPlaytimeTrackingForAllItems();
	//
	// // parent-child relationship or dependency management
	// CALL_RESULT( AddUGCDependencyResult_t )
	// virtual SteamAPICall_t AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );
	// CALL_RESULT( RemoveUGCDependencyResult_t )
	// virtual SteamAPICall_t RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );
	//
	// // add/remove app dependence/requirements (usually DLC)
	// CALL_RESULT( AddAppDependencyResult_t )
	// virtual SteamAPICall_t AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID );
	// CALL_RESULT( RemoveAppDependencyResult_t )
	// virtual SteamAPICall_t RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID );
	// // request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
	// // until all app dependencies have been returned
	// CALL_RESULT( GetAppDependenciesResult_t )
	// virtual SteamAPICall_t GetAppDependencies( PublishedFileId_t nPublishedFileID );
	//
	// // delete the item without prompting the user
	// CALL_RESULT( DeleteItemResult_t )
	// virtual SteamAPICall_t DeleteItem( PublishedFileId_t nPublishedFileID );
};

}

#endif
