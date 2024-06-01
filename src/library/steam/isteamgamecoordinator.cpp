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

#include "isteamgamecoordinator.h"

#include "logging.h"

namespace libtas {

EGCResults ISteamGameCoordinator::SendMessage( uint32_t unMsgType, const void *pubData, uint32_t cubData )
{
    LOGTRACE(LCF_STEAM);
	return 0; // k_EGCResultOK
}

bool ISteamGameCoordinator::IsMessageAvailable( uint32_t *pcubMsgSize )
{
    LOGTRACE(LCF_STEAM);
	return false;
}

EGCResults ISteamGameCoordinator::RetrieveMessage( uint32_t *punMsgType, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize )
{
    LOGTRACE(LCF_STEAM);
	return 1; // k_EGCResultNoMessage
}

}
