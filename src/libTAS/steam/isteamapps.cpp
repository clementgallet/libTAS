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

#include "isteamapps.h"
#include "../logging.h"

namespace libtas {

bool ISteamApps::BIsSubscribed()
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamApps::BIsLowViolence()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsCybercafe()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

bool ISteamApps::BIsVACBanned()
{
    DEBUGLOGCALL(LCF_STEAM);
	return false;
}

const char *ISteamApps::GetCurrentGameLanguage()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "english";
}

const char *ISteamApps::GetAvailableGameLanguages()
{
    DEBUGLOGCALL(LCF_STEAM);
	return "english";
}

bool ISteamApps::BIsSubscribedApp( AppId_t appID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

}
