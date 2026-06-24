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

#ifndef LIBTAS_FMODSTUDIO_H_INCL
#define LIBTAS_FMODSTUDIO_H_INCL

#include "hook.h"
#include "fmod.h"

namespace libtas {

typedef struct FMOD_STUDIO_SYSTEM FMOD_STUDIO_SYSTEM;
typedef unsigned int FMOD_STUDIO_INITFLAGS;

#define FMOD_STUDIO_INIT_NORMAL                0x00000000
#define FMOD_STUDIO_INIT_LIVEUPDATE            0x00000001
#define FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS 0x00000002
#define FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE    0x00000004
#define FMOD_STUDIO_INIT_DEFERRED_CALLBACKS    0x00000008
#define FMOD_STUDIO_INIT_LOAD_FROM_UPDATE      0x00000010
#define FMOD_STUDIO_INIT_MEMORY_TRACKING       0x00000020

OVERRIDE FMOD_RESULT FMOD_Studio_System_Initialize(FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata);
OVERRIDE FMOD_RESULT _ZN4FMOD6Studio6System10initializeEijjPv(FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata);

}

#endif
