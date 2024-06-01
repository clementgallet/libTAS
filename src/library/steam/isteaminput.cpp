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

#include "isteaminput.h"

#include "logging.h"

namespace libtas {

bool ISteamInput::Init( bool bExplicitlyCallRunFrame )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamInput::Shutdown()
{
    LOGTRACE(LCF_STEAM);
    return true;
}
	
bool ISteamInput::SetInputActionManifestFilePath( const char *pchInputActionManifestAbsolutePath )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

void ISteamInput::RunFrame( bool bReservedValue )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamInput::BWaitForData( bool bWaitForever, uint32_t unTimeout )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

bool ISteamInput::BNewDataAvailable()
{
    LOGTRACE(LCF_STEAM);
    return false;
}

int ISteamInput::GetConnectedControllers( InputHandle_t *handlesOut )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
void ISteamInput::EnableDeviceCallbacks()
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback )
{
    LOGTRACE(LCF_STEAM);
}

InputActionSetHandle_t ISteamInput::GetActionSetHandle( const char *pszActionSetName )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
void ISteamInput::ActivateActionSet( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle )
{
    LOGTRACE(LCF_STEAM);
}

InputActionSetHandle_t ISteamInput::GetCurrentActionSet( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamInput::ActivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::DeactivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::DeactivateAllActionSetLayers( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
}

int ISteamInput::GetActiveActionSetLayers( InputHandle_t inputHandle, InputActionSetHandle_t *handlesOut )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

InputDigitalActionHandle_t ISteamInput::GetDigitalActionHandle( const char *pszActionName )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
InputDigitalActionData_t ISteamInput::GetDigitalActionData( InputHandle_t inputHandle, InputDigitalActionHandle_t digitalActionHandle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
int ISteamInput::GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin *originsOut )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
const char *ISteamInput::GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

InputAnalogActionHandle_t ISteamInput::GetAnalogActionHandle( const char *pszActionName )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}
	
InputAnalogActionData_t ISteamInput::GetAnalogActionData( InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamInput::GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin *originsOut )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

const char *ISteamInput::GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32_t unFlags )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

const char *ISteamInput::GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32_t unFlags )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

const char *ISteamInput::GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin )
{
    LOGTRACE(LCF_STEAM);
    return "";
}
	
const char *ISteamInput::GetStringForActionOrigin( EInputActionOrigin eOrigin )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

const char *ISteamInput::GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

void ISteamInput::StopAnalogActionMomentum( InputHandle_t inputHandle, InputAnalogActionHandle_t eAction )
{
    LOGTRACE(LCF_STEAM);
}

InputMotionData_t ISteamInput::GetMotionData( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamInput::TriggerVibration( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8_t nIntensity, char nGainDB, uint8_t nOtherIntensity, char nOtherGainDB )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::SetLEDColor( InputHandle_t inputHandle, uint8_t nColorR, uint8_t nColorG, uint8_t nColorB, unsigned int nFlags )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    LOGTRACE(LCF_STEAM);
}

void ISteamInput::Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamInput::ShowBindingPanel( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
    return true;
}

ESteamInputType ISteamInput::GetInputTypeForHandle( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
    return k_ESteamInputType_Unknown;
}

InputHandle_t ISteamInput::GetControllerForGamepadIndex( int nIndex )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

int ISteamInput::GetGamepadIndexForController( InputHandle_t ulinputHandle )
{
    LOGTRACE(LCF_STEAM);
    return -1;
}
	
const char *ISteamInput::GetStringForXboxOrigin( EXboxOrigin eOrigin )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

const char *ISteamInput::GetGlyphForXboxOrigin( EXboxOrigin eOrigin )
{
    LOGTRACE(LCF_STEAM);
    return "";
}

EInputActionOrigin ISteamInput::GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin )
{
    LOGTRACE(LCF_STEAM);
    return k_EInputActionOrigin_None;
}

EInputActionOrigin ISteamInput::TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin )
{
    LOGTRACE(LCF_STEAM);
    return k_EInputActionOrigin_None;
}

bool ISteamInput::GetDeviceBindingRevision( InputHandle_t inputHandle, int *pMajor, int *pMinor )
{
    LOGTRACE(LCF_STEAM);
    return false;
}

uint32_t ISteamInput::GetRemotePlaySessionID( InputHandle_t inputHandle )
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

uint16_t ISteamInput::GetSessionInputConfigurationSettings()
{
    LOGTRACE(LCF_STEAM);
    return 0;
}

void ISteamInput::SetDualSenseTriggerEffect( InputHandle_t inputHandle, const ScePadTriggerEffectParam *pParam )
{
    LOGTRACE(LCF_STEAM);
}

}
