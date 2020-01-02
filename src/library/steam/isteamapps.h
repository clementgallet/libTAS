//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMAPPS_H_INCL
#define LIBTAS_ISTEAMAPPS_H_INCL

#include <stdint.h>
#include "steamtypes.h"

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: interface to app data
//-----------------------------------------------------------------------------
class ISteamApps
{
public:
	virtual bool BIsSubscribed();
	virtual bool BIsLowViolence();
	virtual bool BIsCybercafe();
	virtual bool BIsVACBanned();
	virtual const char *GetCurrentGameLanguage();
	virtual const char *GetAvailableGameLanguages();

	// only use this member if you need to check ownership of another game related to yours, a demo for example
	virtual bool BIsSubscribedApp( AppId_t appID );

	// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
	virtual bool BIsDlcInstalled( AppId_t appID );

	// returns the Unix time of the purchase of the app
	virtual unsigned int GetEarliestPurchaseUnixTime( AppId_t nAppID );

	// Checks if the user is subscribed to the current app through a free weekend
	// This function will return false for users who have a retail or other type of license
	// Before using, please ask your Valve technical contact how to package and secure your free weekened
	virtual bool BIsSubscribedFromFreeWeekend();

	// Returns the number of DLC pieces for the running app
	virtual int GetDLCCount();

	// Returns metadata for DLC by index, of range [0, GetDLCCount()]
	virtual bool BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize );

	// Install/Uninstall control for optional DLC
	virtual void InstallDLC( AppId_t nAppID );
	virtual void UninstallDLC( AppId_t nAppID );

	// Request legacy cd-key for yourself or owned DLC. If you are interested in this
	// data then make sure you provide us with a list of valid keys to be distributed
	// to users when they purchase the game, before the game ships.
	// You'll receive an AppProofOfPurchaseKeyResponse_t callback when
	// the key is available (which may be immediately).
	virtual void RequestAppProofOfPurchaseKey( AppId_t nAppID );

	virtual bool GetCurrentBetaName( char *pchName, int cchNameBufferSize ); // returns current beta branch name, 'public' is the default branch
	virtual bool MarkContentCorrupt( bool bMissingFilesOnly ); // signal Steam that game files seems corrupt or missing
	virtual unsigned int GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, unsigned int cMaxDepots ); // return installed depots in mount order

	// returns current app install folder for AppID, returns folder name length
	virtual unsigned int GetAppInstallDir( AppId_t appID, char *pchFolder, unsigned int cchFolderBufferSize );
	virtual bool BIsAppInstalled( AppId_t appID ); // returns true if that app is installed (not necessarily owned)

	virtual CSteamID GetAppOwner(); // returns the SteamID of the original owner. If different from current user, it's borrowed

	// Returns the associated launch param if the game is run via steam://run/<appid>//?param1=value1;param2=value2;param3=value3 etc.
	// Parameter names starting with the character '@' are reserved for internal use and will always return and empty string.
	// Parameter names starting with an underscore '_' are reserved for steam features -- they can be queried by the game,
	// but it is advised that you not param names beginning with an underscore for your own features.
	virtual const char *GetLaunchQueryParam( const char *pchKey );

	// get download progress for optional DLC
	virtual bool GetDlcDownloadProgress( AppId_t nAppID, uint64_t *punBytesDownloaded, uint64_t *punBytesTotal );

	// return the buildid of this app, may change at any time based on backend updates to the game
	virtual int GetAppBuildId();

	// Request all proof of purchase keys for the calling appid and asociated DLC.
	// A series of AppProofOfPurchaseKeyResponse_t callbacks will be sent with
	// appropriate appid values, ending with a final callback where the m_nAppId
	// member is k_uAppIdInvalid (zero).
	virtual void RequestAllProofOfPurchaseKeys();

	virtual SteamAPICall_t GetFileDetails( const char* pszFileName );
};

}

#endif
