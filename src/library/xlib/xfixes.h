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

#ifndef LIBTAS_XFIXES_H_INCL
#define LIBTAS_XFIXES_H_INCL

#include "hook.h"
#include <X11/X.h>
#include <X11/Xlib.h>

typedef XID PointerBarrier;

namespace libtas {

OVERRIDE PointerBarrier XFixesCreatePointerBarrier(Display *dpy, Window w, int x1, int y1, int x2, int y2, int directions, int num_devices, int *devices);
OVERRIDE void XFixesDestroyPointerBarrier(Display *dpy, PointerBarrier b);

}

#endif
