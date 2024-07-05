/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_AKAUDIO_H_INCL
#define LIBTAS_AKAUDIO_H_INCL

#include "hook.h"

namespace libtas {

typedef enum AkAudioAPILinux {
    AkAPI_PulseAudio = 1 << 0,                      ///< Use PulseAudio (this is the preferred API on Linux)
    AkAPI_ALSA = 1 << 1,                            ///< Use ALSA
    AkAPI_Default = AkAPI_PulseAudio | AkAPI_ALSA,  ///< Default value, will select the more appropriate API
} AkAudioAPI;

/* I'm just guessing the function arguments from Ghidra analysis */
typedef void AkPlatformInitSettings;
typedef void AkContext;

OVERRIDE AkPlatformInitSettings* CSharp_AkInitializationSettings_platformSettings_get(AkContext *context);
OVERRIDE void CSharp_AkPlatformInitSettings_eAudioAPI_set(AkPlatformInitSettings* settings, AkAudioAPI eAudioAPI);

}

#endif
