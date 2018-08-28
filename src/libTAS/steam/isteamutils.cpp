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

#include "isteamutils.h"
#include "../logging.h"

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

}
