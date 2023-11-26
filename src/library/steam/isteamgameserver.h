//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to steam for game servers
//
//=============================================================================

#ifndef LIBTAS_ISTEAMGAMESERVER_H
#define LIBTAS_ISTEAMGAMESERVER_H

#include "steamtypes.h"

#include <stdint.h>

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: Functions for authenticating users via Steam to play on a game server
//-----------------------------------------------------------------------------
class ISteamGameServer
{
public:

//
// Basic server data.  These properties, if set, must be set before before calling LogOn.  They
// may not be changed after logged in.
//

	/// This is called by SteamGameServer_Init, and you will usually not need to call it directly
	virtual bool InitGameServer( uint32_t unIP, uint16_t usGamePort, uint16_t usQueryPort, uint32_t unFlags, AppId_t nGameAppId, const char *pchVersionString );

	/// Game product identifier.  This is currently used by the master server for version checking purposes.
	/// It's a required field, but will eventually will go away, and the AppID will be used for this purpose.
	virtual void SetProduct( const char *pszProduct );

	/// Description of the game.  This is a required field and is displayed in the steam server browser....for now.
	/// This is a required field, but it will go away eventually, as the data should be determined from the AppID.
	virtual void SetGameDescription( const char *pszGameDescription );

	/// If your game is a "mod," pass the string that identifies it.  The default is an empty string, meaning
	/// this application is the original game, not a mod.
	///
	/// @see k_cbMaxGameServerGameDir
	virtual void SetModDir( const char *pszModDir );

	/// Is this is a dedicated server?  The default value is false.
	virtual void SetDedicatedServer( bool bDedicated );

//
// Login
//

	/// Begin process to login to a persistent game server account
	///
	/// You need to register for callbacks to determine the result of this operation.
	/// @see SteamServersConnected_t
	/// @see SteamServerConnectFailure_t
	/// @see SteamServersDisconnected_t
	virtual void LogOn( const char *pszToken );

	/// Login to a generic, anonymous account.
	///
	/// Note: in previous versions of the SDK, this was automatically called within SteamGameServer_Init,
	/// but this is no longer the case.
	virtual void LogOnAnonymous();

	/// Begin process of logging game server out of steam
	virtual void LogOff();

	// status functions
	virtual bool BLoggedOn();
	virtual bool BSecure();
	virtual CSteamID GetSteamID();

	/// Returns true if the master server has requested a restart.
	/// Only returns true once per request.
	virtual bool WasRestartRequested();

//
// Server state.  These properties may be changed at any time.
//

	/// Max player count that will be reported to server browser and client queries
	virtual void SetMaxPlayerCount( int cPlayersMax );

	/// Number of bots.  Default value is zero
	virtual void SetBotPlayerCount( int cBotplayers );

	/// Set the name of server as it will appear in the server browser
	///
	/// @see k_cbMaxGameServerName
	virtual void SetServerName( const char *pszServerName );

	/// Set name of map to report in the server browser
	///
	/// @see k_cbMaxGameServerName
	virtual void SetMapName( const char *pszMapName );

	/// Let people know if your server will require a password
	virtual void SetPasswordProtected( bool bPasswordProtected );

	/// Spectator server.  The default value is zero, meaning the service
	/// is not used.
	virtual void SetSpectatorPort( uint16_t unSpectatorPort );

	/// Name of the spectator server.  (Only used if spectator port is nonzero.)
	///
	/// @see k_cbMaxGameServerMapName
	virtual void SetSpectatorServerName( const char *pszSpectatorServerName );

	/// Call this to clear the whole list of key/values that are sent in rules queries.
	virtual void ClearAllKeyValues();

	/// Call this to add/update a key/value pair.
	virtual void SetKeyValue( const char *pKey, const char *pValue );

	/// Sets a string defining the "gametags" for this server, this is optional, but if it is set
	/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
	///
	/// @see k_cbMaxGameServerTags
	virtual void SetGameTags( const char *pchGameTags );

	/// Sets a string defining the "gamedata" for this server, this is optional, but if it is set
	/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
	/// don't set this unless it actually changes, its only uploaded to the master once (when
	/// acknowledged)
	///
	/// @see k_cbMaxGameServerGameData
	virtual void SetGameData( const char *pchGameData );

	/// Region identifier.  This is an optional field, the default value is empty, meaning the "world" region
	virtual void SetRegion( const char *pszRegion );

//
// Player list management / authentication
//

	// Handles receiving a new connection from a Steam user.  This call will ask the Steam
	// servers to validate the users identity, app ownership, and VAC status.  If the Steam servers
	// are off-line, then it will validate the cached ticket itself which will validate app ownership
	// and identity.  The AuthBlob here should be acquired on the game client using SteamUser()->InitiateGameConnection()
	// and must then be sent up to the game server for authentication.
	//
	// Return Value: returns true if the users ticket passes basic checks. pSteamIDUser will contain the Steam ID of this user. pSteamIDUser must NOT be NULL
	// If the call succeeds then you should expect a GSClientApprove_t or GSClientDeny_t callback which will tell you whether authentication
	// for the user has succeeded or failed (the steamid in the callback will match the one returned by this call)
	virtual bool SendUserConnectAndAuthenticate( uint32_t unIPClient, const void *pvAuthBlob, uint32_t cubAuthBlobSize, CSteamID *pSteamIDUser );

	// Creates a fake user (ie, a bot) which will be listed as playing on the server, but skips validation.
	//
	// Return Value: Returns a SteamID for the user to be tracked with, you should call HandleUserDisconnect()
	// when this user leaves the server just like you would for a real user.
	virtual CSteamID CreateUnauthenticatedUserConnection();

	// Should be called whenever a user leaves our game server, this lets Steam internally
	// track which users are currently on which servers for the purposes of preventing a single
	// account being logged into multiple servers, showing who is currently on a server, etc.
	virtual void SendUserDisconnect( CSteamID steamIDUser );

	// Update the data to be displayed in the server browser and matchmaking interfaces for a user
	// currently connected to the server.  For regular users you must call this after you receive a
	// GSUserValidationSuccess callback.
	//
	// Return Value: true if successful, false if failure (ie, steamIDUser wasn't for an active player)
	virtual bool BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32_t uScore );

	// New auth system APIs - do not mix with the old auth system APIs.
	// ----------------------------------------------------------------

	// Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ).
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32_t *pcbTicket );

	// Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID );

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( HAuthTicket hAuthTicket );

	// After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID );

	// Ask if a user in in the specified group, results returns async by GSUserGroupStatus_t
	// returns false if we're not connected to the steam servers and thus cannot ask
	virtual bool RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup );


	// these two functions s are deprecated, and will not return results
	// they will be removed in a future version of the SDK
	virtual void GetGameplayStats( );

	virtual SteamAPICall_t GetServerReputation();

	// Returns the public IP of the server according to Steam, useful when the server is
	// behind NAT and you want to advertise its IP in a lobby for other clients to directly
	// connect to
	virtual uint32_t GetPublicIP();

// These are in GameSocketShare mode, where instead of ISteamGameServer creating its own
// socket to talk to the master server on, it lets the game use its socket to forward messages
// back and forth. This prevents us from requiring server ops to open up yet another port
// in their firewalls.
//
// the IP address and port should be in host order, i.e 127.0.0.1 == 0x7f000001

	// These are used when you've elected to multiplex the game server's UDP socket
	// rather than having the master server updater use its own sockets.
	//
	// Source games use this to simplify the job of the server admins, so they
	// don't have to open up more ports on their firewalls.

	// Call this when a packet that starts with 0xFFFFFFFF comes in. That means
	// it's for us.
	virtual bool HandleIncomingPacket( const void *pData, int cbData, uint32_t srcIP, uint16_t srcPort );

	// AFTER calling HandleIncomingPacket for any packets that came in that frame, call this.
	// This gets a packet that the master server updater needs to send out on UDP.
	// It returns the length of the packet it wants to send, or 0 if there are no more packets to send.
	// Call this each frame until it returns 0.
	virtual int GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32_t *pNetAdr, uint16_t *pPort );

//
// Control heartbeats / advertisement with master server
//

	// Call this as often as you like to tell the master server updater whether or not
	// you want it to be active (default: off).
	virtual void EnableHeartbeats( bool bActive );

	// You usually don't need to modify this.
	// Pass -1 to use the default value for iHeartbeatInterval.
	// Some mods change this.
	virtual void SetHeartbeatInterval( int iHeartbeatInterval );

	// Force a heartbeat to steam at the next opportunity
	virtual void ForceHeartbeat();

	// associate this game server with this clan for the purposes of computing player compat
	virtual SteamAPICall_t AssociateWithClan( CSteamID steamIDClan );

	// ask if any of the current players dont want to play with this new player - or vice versa
	virtual SteamAPICall_t ComputeNewPlayerCompatibility( CSteamID steamIDNewPlayer );

};

}

#endif // ISTEAMGAMESERVER_H
