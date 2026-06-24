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

#include "fmodstudio.h"

#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(FMOD_Studio_System_Initialize)
DEFINE_ORIG_POINTER(_ZN4FMOD6Studio6System10initializeEijjPv)

/* Override */ FMOD_RESULT FMOD_Studio_System_Initialize(FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata)
{
    LOGTRACE(LCF_SOUND, "%s call with maxchannels %d, studioflags %x, flags %x", __func__, maxchannels, studioflags, flags);
    LINK_NAMESPACE(FMOD_Studio_System_Initialize, "fmodstudio");
    return orig::FMOD_Studio_System_Initialize(system, maxchannels, studioflags | FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE | FMOD_STUDIO_INIT_DEFERRED_CALLBACKS | FMOD_STUDIO_INIT_LOAD_FROM_UPDATE, flags, extradriverdata);
}

 /* Override */ FMOD_RESULT _ZN4FMOD6Studio6System10initializeEijjPv(FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata)
{
    LOGTRACE(LCF_SOUND, "FMOD::Studio::System::initialize call with maxchannels %d, studioflags %x, flags %x", maxchannels, studioflags, flags);
    LINK_NAMESPACE(_ZN4FMOD6Studio6System10initializeEijjPv, "fmodstudio");
    return orig::_ZN4FMOD6Studio6System10initializeEijjPv(system, maxchannels, studioflags | FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE | FMOD_STUDIO_INIT_DEFERRED_CALLBACKS | FMOD_STUDIO_INIT_LOAD_FROM_UPDATE, flags, extradriverdata);
}

}
