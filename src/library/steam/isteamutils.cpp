/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "isteamutils.h"

#include "logging.h"

namespace libtas {

unsigned int ISteamUtils::GetSecondsSinceAppActive()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

unsigned int ISteamUtils::GetSecondsSinceComputerActive()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

EUniverse ISteamUtils::GetConnectedUniverse()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1; // k_EUniversePublic
}

unsigned int ISteamUtils::GetServerRealTime()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

const char *ISteamUtils::GetIPCountry()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "US";
}

bool ISteamUtils::GetImageSize( int iImage, unsigned int *pnWidth, unsigned int *pnHeight )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamUtils::GetImageRGBA( int iImage, uint8_t *pubDest, int nDestBufferSize )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamUtils::GetCSERIPPort( unsigned int *unIP, uint16_t *usPort )
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

	// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
uint8_t ISteamUtils::GetCurrentBatteryPower()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 255;
}

unsigned int ISteamUtils::GetAppID()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1234;
}

void ISteamUtils::SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamUtils::IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

ESteamAPICallFailure ISteamUtils::GetAPICallFailureReason( SteamAPICall_t hSteamAPICall )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

bool ISteamUtils::GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

void ISteamUtils::RunFrame()
{
    DEBUGLOGCALL(LCF_STEAM);
}

unsigned int ISteamUtils::GetIPCCallCount()
{
    DEBUGLOGCALL(LCF_STEAM);
	return 0;
}

void ISteamUtils::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamUtils::IsOverlayEnabled()
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamUtils::BOverlayNeedsPresent()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

DUMMY_STEAM_IMPL(ISteamUtils::CheckFileSignature, (const char *szFileName), SteamAPICall_t, 1)
DUMMY_STEAM_IMPL(ISteamUtils::ShowGamepadTextInput, ( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, unsigned int unCharMax, const char *pchExistingText ), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::GetEnteredGamepadTextLength, (), unsigned int, 0)
DUMMY_STEAM_IMPL(ISteamUtils::GetEnteredGamepadTextInput, ( char *pchText, unsigned int cchText ), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::GetSteamUILanguage, (), const char *, "")
DUMMY_STEAM_IMPL(ISteamUtils::IsSteamRunningInVR, (), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::SetOverlayNotificationInset, ( int nHorizontalInset, int nVerticalInset ), void, )
DUMMY_STEAM_IMPL(ISteamUtils::IsSteamInBigPictureMode, (), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::StartVRDashboard, (), void, )
DUMMY_STEAM_IMPL(ISteamUtils::IsVRHeadsetStreamingEnabled, (), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::SetVRHeadsetStreamingEnabled, (bool bEnabled), void,)
DUMMY_STEAM_IMPL(ISteamUtils::IsSteamChinaLauncher, (), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::InitFilterText, (uint32_t unFilterOptions), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::FilterText, ( ETextFilteringContext eContext, CSteamID sourceSteamID, const char *pchInputMessage, char *pchOutFilteredText, uint32_t nByteSizeOutFilteredText ), int, 0)
DUMMY_STEAM_IMPL(ISteamUtils::GetIPv6ConnectivityState, ( ESteamIPv6ConnectivityProtocol eProtocol ), ESteamIPv6ConnectivityState, 0)
DUMMY_STEAM_IMPL(ISteamUtils::IsSteamRunningOnSteamDeck, (), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::ShowFloatingGamepadTextInput, ( EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight ), bool, false)
DUMMY_STEAM_IMPL(ISteamUtils::SetGameLauncherMode, (bool bLauncherMode), void, )
DUMMY_STEAM_IMPL(ISteamUtils::DismissFloatingGamepadTextInput, (), bool, false)

}
