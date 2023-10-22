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

#ifndef LIBTAS_XRANDR_H_INCL
#define LIBTAS_XRANDR_H_INCL

#include "../hook.h"
#include "../../external/X11/Xrandr.h"
#include <X11/X.h>

namespace libtas {

OVERRIDE XRRScreenResources *XRRGetScreenResourcesCurrent (Display *dpy, Window window);
OVERRIDE XRRScreenResources *XRRGetScreenResources (Display *dpy, Window window);
OVERRIDE void XRRFreeScreenResources (XRRScreenResources *resources);

OVERRIDE XRROutputInfo *XRRGetOutputInfo (Display *dpy, XRRScreenResources *resources, RROutput output);
OVERRIDE void XRRFreeOutputInfo (XRROutputInfo *outputInfo);

OVERRIDE XRRCrtcInfo *XRRGetCrtcInfo (Display *dpy, XRRScreenResources *resources, RRCrtc crtc);
OVERRIDE void XRRFreeCrtcInfo (XRRCrtcInfo *crtcInfo);

OVERRIDE Atom *XRRListOutputProperties (Display *dpy, RROutput output, int *nprop);

OVERRIDE Status XRRSetCrtcConfig (Display *dpy, XRRScreenResources *resources, RRCrtc crtc, Time timestamp, int x, int y, RRMode mode, Rotation rotation, RROutput *outputs, int noutputs);
OVERRIDE Status XRRSetScreenConfig (Display *dpy, XRRScreenConfiguration *config, Drawable draw, int size_index, Rotation rotation, Time timestamp);
OVERRIDE Status XRRSetScreenConfigAndRate (Display *dpy, XRRScreenConfiguration *config, Drawable draw, int size_index, Rotation rotation, short rate, Time timestamp);
OVERRIDE void XRRSetScreenSize (Display *dpy, Window window, int width, int height, int mmWidth, int mmHeight);

/* Get the first monitor resolution to simulate fullscreen switch while
 * keeping windowed mode */
void get_monitor_resolution(int& width, int& height);

}

#endif
