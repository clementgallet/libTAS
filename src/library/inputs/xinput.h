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

#ifndef LIBTAS_XINPUT_H_INCL
#define LIBTAS_XINPUT_H_INCL

#include "../hook.h"
#include "../../external/X11/XInput2.h"

namespace libtas {

extern int xinput_opcode;

OVERRIDE int XISelectEvents(Display* dpy, Window win, XIEventMask *masks, int num_masks);
OVERRIDE XIDeviceInfo* XIQueryDevice(Display* dpy, int deviceid, int* ndevices_return);
OVERRIDE void XIFreeDeviceInfo( XIDeviceInfo *info);

}

#endif
