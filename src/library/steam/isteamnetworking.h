//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to steam managing network connections between game clients & servers
//
//=============================================================================

#ifndef LIBTAS_ISTEAMNETWORKING_H_INCL
#define LIBTAS_ISTEAMNETWORKING_H_INCL

#include <stdint.h>
#include "steamtypes.h"

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: Functions for making connections and sending data between clients,
//			traversing NAT's where possible
//-----------------------------------------------------------------------------
class ISteamNetworking
{
public:
	////////////////////////////////////////////////////////////////////////////////////////////
	// Session-less connection functions
	//    automatically establishes NAT-traversing or Relay server connections

	// Sends a P2P packet to the specified user
	// UDP-like, unreliable and a max packet size of 1200 bytes
	// the first packet send may be delayed as the NAT-traversal code runs
	// if we can't get through to the user, an error will be posted via the callback P2PSessionConnectFail_t
	// see EP2PSend enum above for the descriptions of the different ways of sending packets
	//
	// nChannel is a routing number you can use to help route message to different systems 	- you'll have to call ReadP2PPacket() 
	// with the same channel number in order to retrieve the data on the other end
	// using different channels to talk to the same user will still use the same underlying p2p connection, saving on resources
	virtual bool SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32_t cubData, EP2PSend eP2PSendType, int nChannel = 0 );

	// returns true if any data is available for read, and the amount of data that will need to be read
	virtual bool IsP2PPacketAvailable( uint32_t *pcubMsgSize, int nChannel = 0 );

	// reads in a packet that has been sent from another user via SendP2PPacket()
	// returns the size of the message and the steamID of the user who sent it in the last two parameters
	// if the buffer passed in is too small, the message will be truncated
	// this call is not blocking, and will return false if no data is available
	virtual bool ReadP2PPacket( void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel = 0 );

	// AcceptP2PSessionWithUser() should only be called in response to a P2PSessionRequest_t callback
	// P2PSessionRequest_t will be posted if another user tries to send you a packet that you haven't talked to yet
	// if you don't want to talk to the user, just ignore the request
	// if the user continues to send you packets, another P2PSessionRequest_t will be posted periodically
	// this may be called multiple times for a single user
	// (if you've called SendP2PPacket() on the other user, this implicitly accepts the session request)
	virtual bool AcceptP2PSessionWithUser( CSteamID steamIDRemote );

	// call CloseP2PSessionWithUser() when you're done talking to a user, will free up resources under-the-hood
	// if the remote user tries to send data to you again, another P2PSessionRequest_t callback will be posted
	virtual bool CloseP2PSessionWithUser( CSteamID steamIDRemote );

	// call CloseP2PChannelWithUser() when you're done talking to a user on a specific channel. Once all channels
	// open channels to a user have been closed, the open session to the user will be closed and new data from this
	// user will trigger a P2PSessionRequest_t callback
	virtual bool CloseP2PChannelWithUser( CSteamID steamIDRemote, int nChannel );

	// fills out P2PSessionState_t structure with details about the underlying connection to the user
	// should only needed for debugging purposes
	// returns false if no connection exists to the specified user
	virtual bool GetP2PSessionState( CSteamID steamIDRemote, P2PSessionState_t *pConnectionState );

	// Allow P2P connections to fall back to being relayed through the Steam servers if a direct connection
	// or NAT-traversal cannot be established. Only applies to connections created after setting this value,
	// or to existing connections that need to automatically reconnect after this value is set.
	//
	// P2P packet relay is allowed by default
	virtual bool AllowP2PPacketRelay( bool bAllow );


	////////////////////////////////////////////////////////////////////////////////////////////
	// LISTEN / CONNECT style interface functions
	//
	// This is an older set of functions designed around the Berkeley TCP sockets model
	// it's preferential that you use the above P2P functions, they're more robust
	// and these older functions will be removed eventually
	//
	////////////////////////////////////////////////////////////////////////////////////////////


	// creates a socket and listens others to connect
	// will trigger a SocketStatusCallback_t callback on another client connecting
	// nVirtualP2PPort is the unique ID that the client will connect to, in case you have multiple ports
	//		this can usually just be 0 unless you want multiple sets of connections
	// unIP is the local IP address to bind to
	//		pass in 0 if you just want the default local IP
	// unPort is the port to use
	//		pass in 0 if you don't want users to be able to connect via IP/Port, but expect to be always peer-to-peer connections only
	virtual SNetListenSocket_t CreateListenSocket( int nVirtualP2PPort, uint32_t nIP, uint16_t nPort, bool bAllowUseOfPacketRelay );

	// creates a socket and begin connection to a remote destination
	// can connect via a known steamID (client or game server), or directly to an IP
	// on success will trigger a SocketStatusCallback_t callback
	// on failure or timeout will trigger a SocketStatusCallback_t callback with a failure code in m_eSNetSocketState
	virtual SNetSocket_t CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay );
	virtual SNetSocket_t CreateConnectionSocket( uint32_t nIP, uint16_t nPort, int nTimeoutSec );

	// disconnects the connection to the socket, if any, and invalidates the handle
	// any unread data on the socket will be thrown away
	// if bNotifyRemoteEnd is set, socket will not be completely destroyed until the remote end acknowledges the disconnect
	virtual bool DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd );
	// destroying a listen socket will automatically kill all the regular sockets generated from it
	virtual bool DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd );

	// sending data
	// must be a handle to a connected socket
	// data is all sent via UDP, and thus send sizes are limited to 1200 bytes; after this, many routers will start dropping packets
	// use the reliable flag with caution; although the resend rate is pretty aggressive,
	// it can still cause stalls in receiving data (like TCP)
	virtual bool SendDataOnSocket( SNetSocket_t hSocket, void *pubData, uint32_t cubData, bool bReliable );

	// receiving data
	// returns false if there is no data remaining
	// fills out *pcubMsgSize with the size of the next message, in bytes
	virtual bool IsDataAvailableOnSocket( SNetSocket_t hSocket, uint32_t *pcubMsgSize ); 

	// fills in pubDest with the contents of the message
	// messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
	// if *pcubMsgSize < cubDest, only partial data is written
	// returns false if no data is available
	virtual bool RetrieveDataFromSocket( SNetSocket_t hSocket, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize ); 

	// checks for data from any socket that has been connected off this listen socket
	// returns false if there is no data remaining
	// fills out *pcubMsgSize with the size of the next message, in bytes
	// fills out *phSocket with the socket that data is available on
	virtual bool IsDataAvailable( SNetListenSocket_t hListenSocket, uint32_t *pcubMsgSize, SNetSocket_t *phSocket );

	// retrieves data from any socket that has been connected off this listen socket
	// fills in pubDest with the contents of the message
	// messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
	// if *pcubMsgSize < cubDest, only partial data is written
	// returns false if no data is available
	// fills out *phSocket with the socket that data is available on
	virtual bool RetrieveData( SNetListenSocket_t hListenSocket, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, SNetSocket_t *phSocket );

	// returns information about the specified socket, filling out the contents of the pointers
	virtual bool GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32_t *punIPRemote, uint16_t *punPortRemote );

	// returns which local port the listen socket is bound to
	// *pnIP and *pnPort will be 0 if the socket is set to listen for P2P connections only
	virtual bool GetListenSocketInfo( SNetListenSocket_t hListenSocket, uint32_t *pnIP, uint16_t *pnPort );

	// returns true to describe how the socket ended up connecting
	virtual ESNetSocketConnectionType GetSocketConnectionType( SNetSocket_t hSocket );

	// max packet size, in bytes
	virtual int GetMaxPacketSize( SNetSocket_t hSocket );
};

// // callback notification - a user wants to talk to us over the P2P channel via the SendP2PPacket() API
// // in response, a call to AcceptP2PPacketsFromUser() needs to be made, if you want to talk with them
// struct P2PSessionRequest_t
// { 
// 	enum { k_iCallback = k_iSteamNetworkingCallbacks + 2 };
// 	CSteamID m_steamIDRemote;			// user who wants to talk to us
// };
// 
// 
// // callback notification - packets can't get through to the specified user via the SendP2PPacket() API
// // all packets queued packets unsent at this point will be dropped
// // further attempts to send will retry making the connection (but will be dropped if we fail again)
// struct P2PSessionConnectFail_t
// { 
// 	enum { k_iCallback = k_iSteamNetworkingCallbacks + 3 };
// 	CSteamID m_steamIDRemote;			// user we were sending packets to
// 	uint8 m_eP2PSessionError;			// EP2PSessionError indicating why we're having trouble
// };
// 
// 
// // callback notification - status of a socket has changed
// // used as part of the CreateListenSocket() / CreateP2PConnectionSocket() 
// struct SocketStatusCallback_t
// { 
// 	enum { k_iCallback = k_iSteamNetworkingCallbacks + 1 };
// 	SNetSocket_t m_hSocket;				// the socket used to send/receive data to the remote host
// 	SNetListenSocket_t m_hListenSocket;	// this is the server socket that we were listening on; NULL if this was an outgoing connection
// 	CSteamID m_steamIDRemote;			// remote steamID we have connected to, if it has one
// 	int m_eSNetSocketState;				// socket state, ESNetSocketState
// };

}

#endif // ISTEAMNETWORKING
