//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMSCREENSHOTS_H_INCL
#define LIBTAS_ISTEAMSCREENSHOTS_H_INCL

#include "steamtypes.h"

#include <stdint.h>

namespace libtas {

//-----------------------------------------------------------------------------
// Purpose: Functions for adding screenshots to the user's screenshot library
//-----------------------------------------------------------------------------
class ISteamScreenshots
{
public:
	// Writes a screenshot to the user's screenshot library given the raw image data, which must be in RGB format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	virtual ScreenshotHandle WriteScreenshot( void *pubRGB, uint32_t cubRGB, int nWidth, int nHeight );

	// Adds a screenshot to the user's screenshot library from disk.  If a thumbnail is provided, it must be 200 pixels wide and the same aspect ratio
	// as the screenshot, otherwise a thumbnail will be generated if the user uploads the screenshot.  The screenshots must be in either JPEG or TGA format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	// JPEG, TGA, and PNG formats are supported.
	virtual ScreenshotHandle AddScreenshotToLibrary( const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight );

	// Causes the Steam overlay to take a screenshot.  If screenshots are being hooked by the game then a ScreenshotRequested_t callback is sent back to the game instead.
	virtual void TriggerScreenshot();

	// Toggles whether the overlay handles screenshots when the user presses the screenshot hotkey, or the game handles them.  If the game is hooking screenshots,
	// then the ScreenshotRequested_t callback will be sent if the user presses the hotkey, and the game is expected to call WriteScreenshot or AddScreenshotToLibrary
	// in response.
	virtual void HookScreenshots( bool bHook );

	// Sets metadata about a screenshot's location (for example, the name of the map)
	virtual bool SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation );

	// Tags a user as being visible in the screenshot
	virtual bool TagUser( ScreenshotHandle hScreenshot, CSteamID steamID );

	// Tags a published file as being visible in the screenshot
	virtual bool TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID );

	// Returns true if the app has hooked the screenshot
	virtual bool IsScreenshotsHooked();

	// Adds a VR screenshot to the user's screenshot library from disk in the supported type.
	// pchFilename should be the normal 2D image used in the library view
	// pchVRFilename should contain the image that matches the correct type
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	// JPEG, TGA, and PNG formats are supported.
	virtual ScreenshotHandle AddVRScreenshotToLibrary( EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename );
};

}

#endif
