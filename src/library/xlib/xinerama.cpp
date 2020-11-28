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

#include "xinerama.h"

#include "../hook.h"
#include "../logging.h"

namespace libtas {

DEFINE_ORIG_POINTER(XineramaQueryScreens)

XineramaScreenInfo *XineramaQueryScreens(Display *dpy, int *number)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (shared_config.screen_width) {
        *number = 1;
        XineramaScreenInfo *info = static_cast<XineramaScreenInfo*>(malloc(sizeof(XineramaScreenInfo)));
        info->screen_number = 0;
        info->x_org = 0;
        info->y_org = 0;
        info->width = shared_config.screen_width;
        info->height = shared_config.screen_height;
        return info;
    }
    LINK_NAMESPACE(XineramaQueryScreens, "Xinerama");
    return orig::XineramaQueryScreens(dpy, number);
}

}
