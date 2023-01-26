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

#ifndef LIBTAS_FMOD_H_INCL
#define LIBTAS_FMOD_H_INCL

#include "../../hook.h"

namespace libtas {

OVERRIDE int FMOD_System_Create(void **system);

OVERRIDE int FMOD_EventSystem_Create(void **eventsystem);
OVERRIDE int FMOD_EventSystem_GetSystemObject(void *eventsystem, void **system);
OVERRIDE int _ZN4FMOD11EventSystem15getSystemObjectEPPNS_6SystemE(void *eventsystem, void **system);

OVERRIDE int FMOD_System_SetOutput(void *system, int output);
OVERRIDE int _ZN4FMOD6System9setOutputE15FMOD_OUTPUTTYPE(void *system, int output);


}

#endif
