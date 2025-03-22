//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUSER_PRIV_H_INCL
#define LIBTAS_ISTEAMUSER_PRIV_H_INCL

#include "steam/steamtypes.h"

#include <stdint.h>
#include <string>

namespace libtas {

HSteamUser ISteamUser_GetHSteamUser();
bool ISteamUser_BLoggedOn();
CSteamID ISteamUser_GetSteamID();
int ISteamUser_InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, unsigned int unIPServer, uint16_t usPortServer, bool bSecure );
void ISteamUser_TerminateGameConnection( unsigned int unIPServer, uint16_t usPortServer );
void ISteamUser_TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo );
bool ISteamUser_GetUserDataFolder( char *pchBuffer, int cubBuffer );
void ISteamUser_StartVoiceRecording();
void ISteamUser_StopVoiceRecording();
EVoiceResult ISteamUser_GetAvailableVoice( unsigned int *pcbCompressed, unsigned int *pcbUncompressed_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated);
EVoiceResult ISteamUser_GetVoice( bool bWantCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated, unsigned int cbUncompressedDestBufferSize_Deprecated, unsigned int *nUncompressBytesWritten_Deprecated, unsigned int nUncompressedVoiceDesiredSampleRate_Deprecated);
EVoiceResult ISteamUser_DecompressVoice( const void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, unsigned int nDesiredSampleRate );
unsigned int ISteamUser_GetVoiceOptimalSampleRate();
HAuthTicket ISteamUser_GetAuthSessionTicket( void *pTicket, int cbMaxTicket, unsigned int *pcbTicket );
HAuthTicket ISteamUser_GetAuthTicketForWebApi( const char *pchIdentity );
EBeginAuthSessionResult ISteamUser_BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );
void ISteamUser_EndAuthSession( CSteamID steamID );
void ISteamUser_CancelAuthTicket( HAuthTicket hAuthTicket );

}

#endif
