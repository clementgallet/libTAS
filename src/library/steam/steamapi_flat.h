/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_STEAMAPI_FLAT_H_INCL
#define LIBTAS_STEAMAPI_FLAT_H_INCL

#include "isteamuser/isteamuser.h"
#include "isteamuserstats/isteamuserstats.h"
#include "isteamutils.h"
#include "isteamremotestorage/isteamremotestorage.h"
#include "isteamapps.h"
#include "isteamfriends.h"
#include "isteamscreenshots.h"
#include "isteamugc.h"
#include "isteammatchmaking.h"
#include "isteamhttp.h"
#include "isteaminput.h"
#include "isteamnetworking.h"
#include "isteamnetworkingutils.h"
#include "isteamnetworkingsockets.h"
#include "isteamnetworkingmessages.h"

#include "hook.h"

namespace libtas {

OVERRIDE ISteamUtils *SteamAPI_SteamUtils_v010();
OVERRIDE ISteamApps *SteamAPI_SteamApps_v009();
OVERRIDE ISteamFriends *SteamAPI_SteamFriends_v018();
OVERRIDE ISteamScreenshots *SteamAPI_SteamScreenshots_v003();
OVERRIDE ISteamUGC *SteamAPI_SteamUGC_v021();

typedef void ISteamMusic;
typedef void ISteamHTMLSurface;
typedef void ISteamInventory;
typedef void ISteamVideo;
typedef void ISteamParentalSettings;
typedef void ISteamRemotePlay;

OVERRIDE ISteamMatchmaking *SteamAPI_SteamMatchmaking_v009();
OVERRIDE ISteamNetworking *SteamAPI_SteamNetworking_v006();
OVERRIDE ISteamMatchmakingServers *SteamAPI_SteamMatchmakingServers_v002();
OVERRIDE ISteamHTTP *SteamAPI_SteamHTTP_v003();
OVERRIDE ISteamInput *SteamAPI_SteamInput_v006();
OVERRIDE ISteamMusic *SteamAPI_SteamMusic_v001();
OVERRIDE ISteamHTMLSurface *SteamAPI_SteamHTMLSurface_v005();
OVERRIDE ISteamInventory *SteamAPI_SteamInventory_v003();
OVERRIDE ISteamVideo *SteamAPI_SteamVideo_v007();
OVERRIDE ISteamParentalSettings *SteamAPI_SteamParentalSettings_v001();
OVERRIDE ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
OVERRIDE ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v012();
OVERRIDE ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_SteamAPI_v002();
OVERRIDE ISteamRemotePlay *SteamAPI_SteamRemotePlay_v004();


}

#endif
