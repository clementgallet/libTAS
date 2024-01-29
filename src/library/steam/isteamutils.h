//====== Copyright (c) 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef LIBTAS_ISTEAMUTILS_H_INCL
#define LIBTAS_ISTEAMUTILS_H_INCL

#include "steamtypes.h"

#include <stdint.h>

namespace libtas {

enum EFloatingGamepadTextInputMode {};
enum ETextFilteringContext {};
typedef int ESteamIPv6ConnectivityState;
enum ESteamIPv6ConnectivityProtocol {};

//-----------------------------------------------------------------------------
// Purpose: interface to user independent utility functions
//-----------------------------------------------------------------------------
class ISteamUtils
{
public:
	// return the number of seconds since the user
	virtual unsigned int GetSecondsSinceAppActive();
	virtual unsigned int GetSecondsSinceComputerActive();

	// the universe this client is connecting to
	virtual EUniverse GetConnectedUniverse();

	// Steam server time.  Number of seconds since January 1, 1970, GMT (i.e unix time)
	virtual unsigned int GetServerRealTime();

	// returns the 2 digit ISO 3166-1-alpha-2 format country code this client is running in (as looked up via an IP-to-location database)
	// e.g "US" or "UK".
	virtual const char *GetIPCountry();

	// returns true if the image exists, and valid sizes were filled out
	virtual bool GetImageSize( int iImage, unsigned int *pnWidth, unsigned int *pnHeight );

	// returns true if the image exists, and the buffer was successfully filled out
	// results are returned in RGBA format
	// the destination buffer size should be 4 * height * width * sizeof(char)
	virtual bool GetImageRGBA( int iImage, uint8_t *pubDest, int nDestBufferSize );

	// returns the IP of the reporting server for valve - currently only used in Source engine games
	virtual bool GetCSERIPPort( unsigned int *unIP, uint16_t *usPort );

	// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
	virtual uint8_t GetCurrentBatteryPower();

	// returns the appID of the current process
	virtual unsigned int GetAppID();

	// Sets the position where the overlay instance for the currently calling game should show notifications.
	// This position is per-game and if this function is called from outside of a game context it will do nothing.
	virtual void SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition );

	// API asynchronous call results
	// can be used directly, but more commonly used via the callback dispatch API (see steam_api.h)
	virtual bool IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed );
	virtual ESteamAPICallFailure GetAPICallFailureReason( SteamAPICall_t hSteamAPICall );
	virtual bool GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed );

	// Deprecated. Applications should use SteamAPI_RunCallbacks() instead. Game servers do not need to call this function.
	virtual void RunFrame();

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual unsigned int GetIPCCallCount();

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction );

	// Returns true if the overlay is running & the user can access it. The overlay process could take a few seconds to
	// start & hook the game process, so this function will initially return false while the overlay is loading.
	virtual bool IsOverlayEnabled();

	// Normally this call is unneeded if your game has a constantly running frame loop that calls the
	// D3D Present API, or OGL SwapBuffers API every frame.
	//
	// However, if you have a game that only refreshes the screen on an event driven basis then that can break
	// the overlay, as it uses your Present/SwapBuffers calls to drive it's internal frame loop and it may also
	// need to Present() to the screen any time an even needing a notification happens or when the overlay is
	// brought up over the game by a user.  You can use this API to ask the overlay if it currently need a present
	// in that case, and then you can check for this periodically (roughly 33hz is desirable) and make sure you
	// refresh the screen with Present or SwapBuffers to allow the overlay to do it's work.
	virtual bool BOverlayNeedsPresent();

	// // Asynchronous call to check if an executable file has been signed using the public key set on the signing tab
	// // of the partner site, for example to refuse to load modified executable files.
	// // The result is returned in CheckFileSignature_t.
	// //   k_ECheckFileSignatureNoSignaturesFoundForThisApp - This app has not been configured on the signing tab of the partner site to enable this function.
	// //   k_ECheckFileSignatureNoSignaturesFoundForThisFile - This file is not listed on the signing tab for the partner site.
	// //   k_ECheckFileSignatureFileNotFound - The file does not exist on disk.
	// //   k_ECheckFileSignatureInvalidSignature - The file exists, and the signing tab has been set for this file, but the file is either not signed or the signature does not match.
	// //   k_ECheckFileSignatureValidSignature - The file is signed and the signature is valid.
	virtual SteamAPICall_t CheckFileSignature( const char *szFileName );
	//
	// // Activates the Big Picture text input dialog which only supports gamepad input
	virtual bool ShowGamepadTextInput( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, unsigned int unCharMax, const char *pchExistingText );
	//
	// // Returns previously entered text & length
	virtual unsigned int GetEnteredGamepadTextLength();
	virtual bool GetEnteredGamepadTextInput( char *pchText, unsigned int cchText );
	//
	// // returns the language the steam client is running in, you probably want ISteamApps::GetCurrentGameLanguage instead, this is for very special usage cases
	virtual const char *GetSteamUILanguage();
	//
	// // returns true if Steam itself is running in VR mode
	virtual bool IsSteamRunningInVR();
	//
	// // Sets the inset of the overlay notification from the corner specified by SetOverlayNotificationPosition.
	virtual void SetOverlayNotificationInset( int nHorizontalInset, int nVerticalInset );
	//
	// // returns true if Steam & the Steam Overlay are running in Big Picture mode
	// // Games much be launched through the Steam client to enable the Big Picture overlay. During development,
	// // a game can be added as a non-steam game to the developers library to test this feature
	virtual bool IsSteamInBigPictureMode();
	//
	// // ask SteamUI to create and render its OpenVR dashboard
	virtual void StartVRDashboard();
	//
	// // Returns true if the HMD content will be streamed via Steam In-Home Streaming
	virtual bool IsVRHeadsetStreamingEnabled();
	//
	// // Set whether the HMD content will be streamed via Steam In-Home Streaming
	// // If this is set to true, then the scene in the HMD headset will be streamed, and remote input will not be allowed.
	// // If this is set to false, then the application window will be streamed instead, and remote input will be allowed.
	// // The default is true unless "VRHeadsetStreaming" "0" is in the extended appinfo for a game.
	// // (this is useful for games that have asymmetric multiplayer gameplay)
	virtual void SetVRHeadsetStreamingEnabled( bool bEnabled );
    
    // Returns whether this steam client is a Steam China specific client, vs the global client.
    virtual bool IsSteamChinaLauncher();

    // Initializes text filtering, loading dictionaries for the language the game is running in.
    //   unFilterOptions are reserved for future use and should be set to 0
    // Returns false if filtering is unavailable for the game's language, in which case FilterText() will act as a passthrough.
    //
    // Users can customize the text filter behavior in their Steam Account preferences:
    // https://store.steampowered.com/account/preferences#CommunityContentPreferences
    virtual bool InitFilterText( uint32_t unFilterOptions = 0 );

    // Filters the provided input message and places the filtered result into pchOutFilteredText, using legally required filtering and additional filtering based on the context and user settings
    //   eContext is the type of content in the input string
    //   sourceSteamID is the Steam ID that is the source of the input string (e.g. the player with the name, or who said the chat text)
    //   pchInputText is the input string that should be filtered, which can be ASCII or UTF-8
    //   pchOutFilteredText is where the output will be placed, even if no filtering is performed
    //   nByteSizeOutFilteredText is the size (in bytes) of pchOutFilteredText, should be at least strlen(pchInputText)+1
    // Returns the number of characters (not bytes) filtered
    virtual int FilterText( ETextFilteringContext eContext, CSteamID sourceSteamID, const char *pchInputMessage, char *pchOutFilteredText, uint32_t nByteSizeOutFilteredText );

    // Return what we believe your current ipv6 connectivity to "the internet" is on the specified protocol.
    // This does NOT tell you if the Steam client is currently connected to Steam via ipv6.
    virtual ESteamIPv6ConnectivityState GetIPv6ConnectivityState( ESteamIPv6ConnectivityProtocol eProtocol );

    // returns true if currently running on the Steam Deck device
    virtual bool IsSteamRunningOnSteamDeck();

    // Opens a floating keyboard over the game content and sends OS keyboard keys directly to the game.
    // The text field position is specified in pixels relative the origin of the game window and is used to position the floating keyboard in a way that doesn't cover the text field
    virtual bool ShowFloatingGamepadTextInput( EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight );

    // In game launchers that don't have controller support you can call this to have Steam Input translate the controller input into mouse/kb to navigate the launcher
    virtual void SetGameLauncherMode( bool bLauncherMode );

    // Dismisses the floating keyboard.
    virtual bool DismissFloatingGamepadTextInput();
};

}

#endif
