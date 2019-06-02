/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xrandr.h"
#ifdef LIBTAS_HAS_XRANDR

#include "hook.h"
#include "logging.h"

namespace libtas {

DEFINE_ORIG_POINTER(XRRGetCrtcInfo);

OVERRIDE XRRCrtcInfo *XRRGetCrtcInfo (Display *dpy, XRRScreenResources *resources, RRCrtc crtc)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XRRGetCrtcInfo, "Xrandr");

    XRRCrtcInfo *crtcInfo = orig::XRRGetCrtcInfo(dpy, resources, crtc);

    if (shared_config.screen_width) {
        /* Change the settings. */
        crtcInfo->width = shared_config.screen_width;
        crtcInfo->height = shared_config.screen_height;
        for (int i = 0; i < resources->nmode; i++) {
            if (resources->modes[i].width == shared_config.screen_width &&
                resources->modes[i].height == shared_config.screen_height) {
                crtcInfo->mode = i;
                break;
            }
        }
    }

    return crtcInfo;
}

}

#endif
