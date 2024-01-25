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

#include "fmod.h"

#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(FMOD_System_Create)
DEFINE_ORIG_POINTER(FMOD_EventSystem_Create)
DEFINE_ORIG_POINTER(FMOD_EventSystem_GetSystemObject)
DEFINE_ORIG_POINTER(FMOD_System_SetOutput)
DEFINE_ORIG_POINTER(_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE)

int FMOD_System_Create(void **system, int version)
{
    DEBUGLOGCALL(LCF_SOUND);

    LINK_NAMESPACE(FMOD_System_Create, "fmod");
    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    int ret = orig::FMOD_System_Create(system, version);

    /* We force the output to be ALSA */
    orig::FMOD_System_SetOutput(*system, 11 /* FMOD_OUTPUTTYPE_ALSA */);

    return ret;
}

int FMOD_EventSystem_Create(void **eventsystem)
{
    DEBUGLOGCALL(LCF_SOUND);

    LINK_NAMESPACE(FMOD_EventSystem_Create, "fmod");
    LINK_NAMESPACE(FMOD_EventSystem_GetSystemObject, "fmod");
    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    int ret = orig::FMOD_EventSystem_Create(eventsystem);

    /* We force the output to be ALSA */
    void* system;
    orig::FMOD_EventSystem_GetSystemObject(*eventsystem, &system);
    orig::FMOD_System_SetOutput(system, 11 /* FMOD_OUTPUTTYPE_ALSA */);

    return ret;
}

int FMOD_System_SetOutput(void *system, int)
{
    DEBUGLOGCALL(LCF_SOUND);

    LINK_NAMESPACE(FMOD_System_SetOutput, "fmod");

    /* We force the output to be ALSA */
    return orig::FMOD_System_SetOutput(system, 11 /* FMOD_OUTPUTTYPE_ALSA */);
}

int _ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE(void *system, int output)
{
    DEBUGLOGCALL(LCF_SOUND);

    LINK_NAMESPACE(_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE, "fmod");

    /* We force the output to be ALSA */
    return orig::_ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE(system, 11 /* FMOD_OUTPUTTYPE_ALSA */);

}

}
