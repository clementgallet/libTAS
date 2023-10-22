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

#ifndef LIBTAS_XF86VIDMODE_H_INCL
#define LIBTAS_XF86VIDMODE_H_INCL

#include "../hook.h"
#include "../../external/X11/xf86vmode.h"

// #include <X11/Xlib.h>
// #include <X11/Xutil.h> // XSetWMName

namespace libtas {

OVERRIDE Bool XF86VidModeGetModeLine(Display* dpy, int screen, int* dotclock, XF86VidModeModeLine* modeline);

OVERRIDE Bool XF86VidModeGetAllModeLines(Display *dpy, int screen, int *modecount_return, XF86VidModeModeInfo ***modesinfo);

}

#endif
