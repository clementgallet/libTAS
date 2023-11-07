//====== Copyright ©, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to the game coordinator for this application
//
//=============================================================================

#ifndef LIBTAS_ISTEAMGAMECOORDINATOR_H_INCL
#define LIBTAS_ISTEAMGAMECOORDINATOR_H_INCL

#include "steamtypes.h"

#include <stdint.h>

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: Functions for sending and receiving messages from the Game Coordinator
//			for this application
//-----------------------------------------------------------------------------
class ISteamGameCoordinator
{
public:

	// sends a message to the Game Coordinator
	virtual EGCResults SendMessage( uint32_t unMsgType, const void *pubData, uint32_t cubData );

	// returns true if there is a message waiting from the game coordinator
	virtual bool IsMessageAvailable( uint32_t *pcubMsgSize );

	// fills the provided buffer with the first message in the queue and returns k_EGCResultOK or
	// returns k_EGCResultNoMessage if there is no message waiting. pcubMsgSize is filled with the message size.
	// If the provided buffer is not large enough to fit the entire message, k_EGCResultBufferTooSmall is returned
	// and the message remains at the head of the queue.
	virtual EGCResults RetrieveMessage( uint32_t *punMsgType, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize );

};

}

#endif
