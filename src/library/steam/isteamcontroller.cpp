/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "isteamcontroller.h"
#include "../logging.h"

namespace libtas {

bool ISteamController::Init( const char *pchAbsolutePathToControllerConfigVDF )
{
    debuglog(LCF_STEAM, __func__, " called with config path ", pchAbsolutePathToControllerConfigVDF);
    return true;
}

bool ISteamController::Shutdown()
{
    DEBUGLOGCALL(LCF_STEAM);
    return true;
}

void ISteamController::RunFrame()
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamController::GetControllerState( uint32_t unControllerIndex, SteamControllerState_t *pState )
{
    DEBUGLOGCALL(LCF_STEAM);
    return false;
}

void ISteamController::TriggerHapticPulse( uint32_t unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamController::SetOverrideMode( const char *pchMode )
{
    debuglog(LCF_STEAM, __func__, " called with override mode ", pchMode);
}

}
