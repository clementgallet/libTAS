/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "isteamscreenshots.h"
#include "../logging.h"

namespace libtas {

ScreenshotHandle ISteamScreenshots::WriteScreenshot( void *pubRGB, uint32_t cubRGB, int nWidth, int nHeight )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

ScreenshotHandle ISteamScreenshots::AddScreenshotToLibrary( const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

void ISteamScreenshots::TriggerScreenshot()
{
    DEBUGLOGCALL(LCF_STEAM);
}

void ISteamScreenshots::HookScreenshots( bool bHook )
{
    DEBUGLOGCALL(LCF_STEAM);
}

bool ISteamScreenshots::SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::TagUser( ScreenshotHandle hScreenshot, CSteamID steamID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID )
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::IsScreenshotsHooked()
{
    DEBUGLOGCALL(LCF_STEAM);
	return true;
}

ScreenshotHandle ISteamScreenshots::AddVRScreenshotToLibrary( EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename )
{
    DEBUGLOGCALL(LCF_STEAM);
	return 1;
}

}
