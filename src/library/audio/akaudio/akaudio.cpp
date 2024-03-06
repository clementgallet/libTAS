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

#include "akaudio.h"

#include "logging.h"
#include "hook.h"
#include "global.h"

namespace libtas {

DEFINE_ORIG_POINTER(CSharp_AkInitializationSettings_platformSettings_get)
DEFINE_ORIG_POINTER(CSharp_AkPlatformInitSettings_eAudioAPI_set)

AkPlatformInitSettings* CSharp_AkInitializationSettings_platformSettings_get(AkContext *context)
{
    DEBUGLOGCALL(LCF_SOUND);
    LINK_NAMESPACE(CSharp_AkInitializationSettings_platformSettings_get, "AkSoundEngine");
    AkPlatformInitSettings* settings = orig::CSharp_AkInitializationSettings_platformSettings_get(context);

    if (!(Global::game_info.audio & GameInfo::AKAUDIO)) {
        Global::game_info.audio |= GameInfo::AKAUDIO;
        Global::game_info.tosend = true;
    }
    
    LINK_NAMESPACE(CSharp_AkPlatformInitSettings_eAudioAPI_set, "AkSoundEngine");
    orig::CSharp_AkPlatformInitSettings_eAudioAPI_set(settings, AkAPI_ALSA);

    return settings;
}

}
