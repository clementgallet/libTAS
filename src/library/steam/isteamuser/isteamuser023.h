//====== Copyright )(c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSER023_H_INCL
#define LIBTAS_ISTEAMUSER023_H_INCL

#include "isteamuser.h"

#include <stdint.h>
#define STEAMUSER_INTERFACE_VERSION_023 "SteamUser023"

namespace libtas {

struct ISteamUser023Vtbl
{
    HSteamUser (*GetHSteamUser)();
    bool (*BLoggedOn)();
    CSteamID (*GetSteamID)();
    int (*InitiateGameConnection)( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, unsigned int unIPServer, uint16_t usPortServer, bool bSecure );
    void (*TerminateGameConnection)( unsigned int unIPServer, uint16_t usPortServer );
    void (*TrackAppUsageEvent)( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo );
    bool (*GetUserDataFolder)( char *pchBuffer, int cubBuffer );
    void (*StartVoiceRecording)();
    void (*StopVoiceRecording)();
    EVoiceResult (*GetAvailableVoice)( unsigned int *pcbCompressed, unsigned int *pcbUncompressed_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated);
    EVoiceResult (*GetVoice)( bool bWantCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated, unsigned int cbUncompressedDestBufferSize_Deprecated, unsigned int *nUncompressBytesWritten_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated);
    EVoiceResult (*DecompressVoice)( const void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, unsigned int nDesiredSampleRate );
    unsigned int (*GetVoiceOptimalSampleRate)();
    HAuthTicket (*GetAuthSessionTicket)( void *pTicket, int cbMaxTicket, unsigned int *pcbTicket );
    HAuthTicket (*GetAuthTicketForWebApi)( const char *pchIdentity );
    EBeginAuthSessionResult (*BeginAuthSession)( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );
    void (*EndAuthSession)( CSteamID steamID );
    void (*CancelAuthTicket)( HAuthTicket hAuthTicket );
	// EUserHasLicenseForAppResult (*UserHasLicenseForApp)( CSteamID steamID, AppId_t appID );
	// bool (*BIsBehindNAT)();
	// void (*AdvertiseGame)( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer );
	// SteamAPICall_t (*RequestEncryptedAppTicket)( void *pDataToInclude, int cbDataToInclude );
	// bool (*GetEncryptedAppTicket)( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
	// int (*GetGameBadgeLevel)( int nSeries, bool bFoil );
	// int (*GetPlayerSteamLevel)();
	// SteamAPICall_t (*RequestStoreAuthURL)( const char *pchRedirectURL );
	// bool (*BIsPhoneVerified)();
	// bool (*BIsTwoFactorEnabled)();
	// bool (*BIsPhoneIdentifying)();
	// bool (*BIsPhoneRequiringVerification)();
	// SteamAPICall_t (*GetMarketEligibility)();
	// SteamAPICall_t (*GetDurationControl)();
	// bool (*BSetDurationControlOnlineState)( EDurationControlOnlineState eNewState );
};

struct ISteamUser *SteamUser023(void);

}

#endif
