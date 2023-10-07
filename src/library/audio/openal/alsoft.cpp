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

#include "al.h"
#include "alc.h"
#include "alsoft.h"
#include "../../logging.h"

namespace libtas {

/* We need a few functions in order to properly detect OpenAL Soft */

DECLARE_ORIG_POINTER(alcOpenDevice)
DECLARE_ORIG_POINTER(alcCloseDevice)

DECLARE_ORIG_POINTER(alcCreateContext)
DECLARE_ORIG_POINTER(alcMakeContextCurrent)
DECLARE_ORIG_POINTER(alcDestroyContext)

DECLARE_ORIG_POINTER(alGetString)

bool check_al_soft_available()
{
    static int al_soft_available = -1;
    if (al_soft_available == -1) {
        LINK_NAMESPACE_ALSOFT(alcOpenDevice);
        LINK_NAMESPACE_ALSOFT(alcCloseDevice);
        LINK_NAMESPACE_ALSOFT(alcCreateContext);
        LINK_NAMESPACE_ALSOFT(alcMakeContextCurrent);
        LINK_NAMESPACE_ALSOFT(alcDestroyContext);
        LINK_NAMESPACE_ALSOFT(alGetString);

        ALCdevice* device = nullptr;
        ALCcontext* context = nullptr;

        #define CHECKVAL(x) do { \
            if (!(x)) { \
                if (context) { \
                    orig::alcMakeContextCurrent(nullptr); \
                    orig::alcDestroyContext(context); \
                } \
                if (device) { \
                    orig::alcCloseDevice(device); \
                } \
                al_soft_available = 0; \
                return false; \
            } \
        } while (0)

        CHECKVAL(orig::alcOpenDevice && orig::alcCloseDevice && orig::alcCreateContext &&
            orig::alcMakeContextCurrent && orig::alcDestroyContext && orig::alGetString);

        CHECKVAL(device = orig::alcOpenDevice(nullptr));
        CHECKVAL(context = orig::alcCreateContext(device, nullptr));
        CHECKVAL(orig::alcMakeContextCurrent(context) == ALC_TRUE);

        const char* al_renderer = orig::alGetString(AL_RENDERER);
        debuglogstdio(LCF_SOUND, "orig::alcGetString(AL_RENDERER) returned %s", al_renderer ? al_renderer : "<NULL>");
        al_soft_available = al_renderer && strcmp(al_renderer, "OpenAL Soft") == 0;

        orig::alcMakeContextCurrent(nullptr);
        orig::alcDestroyContext(context);
        orig::alcCloseDevice(device);
    }

    return al_soft_available;
}

}
