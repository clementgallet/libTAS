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

#include "isteamscreenshots.h"

#include "logging.h"

namespace libtas {

ScreenshotHandle ISteamScreenshots::WriteScreenshot( void *pubRGB, uint32_t cubRGB, int nWidth, int nHeight )
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

ScreenshotHandle ISteamScreenshots::AddScreenshotToLibrary( const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight )
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

void ISteamScreenshots::TriggerScreenshot()
{
    LOGTRACE(LCF_STEAM);
}

void ISteamScreenshots::HookScreenshots( bool bHook )
{
    LOGTRACE(LCF_STEAM);
}

bool ISteamScreenshots::SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::TagUser( ScreenshotHandle hScreenshot, CSteamID steamID )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID )
{
    LOGTRACE(LCF_STEAM);
	return true;
}

bool ISteamScreenshots::IsScreenshotsHooked()
{
    LOGTRACE(LCF_STEAM);
	return true;
}

ScreenshotHandle ISteamScreenshots::AddVRScreenshotToLibrary( EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename )
{
    LOGTRACE(LCF_STEAM);
	return 1;
}

}
