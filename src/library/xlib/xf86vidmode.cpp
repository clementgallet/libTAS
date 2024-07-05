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

#include "xf86vidmode.h"

#include "hook.h"
#include "logging.h"
#include "global.h"

namespace libtas {

Bool XF86VidModeGetModeLine(Display* dpy, int screen, int* dotclock, XF86VidModeModeLine* modeline)
{
    LOGTRACE(LCF_WINDOW);

    if (Global::shared_config.screen_width) {
        /* Fill the settings, using my monitor as reference, it
         * shouldn't matter really. */
        modeline->hdisplay = Global::shared_config.screen_height;
        modeline->hsyncstart = Global::shared_config.screen_height + 88;
        modeline->hsyncend = Global::shared_config.screen_height + 132;
        modeline->htotal = Global::shared_config.screen_height + 280;
        modeline->hskew = 0;

        modeline->vdisplay = Global::shared_config.screen_width;
        modeline->vsyncstart = Global::shared_config.screen_width + 4;
        modeline->vsyncend = Global::shared_config.screen_width + 9;
        modeline->vtotal = Global::shared_config.screen_width + 45;

        modeline->flags = 5;
        modeline->privsize = 0;
        
        return True;
    }

    RETURN_NATIVE(XF86VidModeGetModeLine, (dpy, screen, dotclock, modeline), "libXxf86vm.so.1");
}

Bool XF86VidModeGetAllModeLines(Display *dpy, int screen, int *modecount_return, XF86VidModeModeInfo ***modesinfo)
{
    LOGTRACE(LCF_WINDOW);

    if (Global::shared_config.screen_width) {
        /* If I understand correctly the doc, each individual XF86VidModeModeInfo
         * struct are not allocated, but the array is. So we make our
         * fake XF86VidModeModeInfo struct static, and allocate a 1-size array. */

        static XF86VidModeModeInfo modeinfo;
        XF86VidModeModeInfo **modeinfoarr = static_cast<XF86VidModeModeInfo**>(malloc(1*sizeof(XF86VidModeModeInfo*)));
        modeinfoarr[0] = &modeinfo;

        /* Fill the settings, using my monitor as reference, it
         * shouldn't matter really. */
        modeinfo.dotclock = 148500;
        
        modeinfo.hdisplay = Global::shared_config.screen_height;
        modeinfo.hsyncstart = Global::shared_config.screen_height + 88;
        modeinfo.hsyncend = Global::shared_config.screen_height + 132;
        modeinfo.htotal = Global::shared_config.screen_height + 280;
        modeinfo.hskew = 0;

        modeinfo.vdisplay = Global::shared_config.screen_width;
        modeinfo.vsyncstart = Global::shared_config.screen_width + 4;
        modeinfo.vsyncend = Global::shared_config.screen_width + 9;
        modeinfo.vtotal = Global::shared_config.screen_width + 45;

        modeinfo.flags = 5;
        modeinfo.privsize = 0;

        /* Set the correct returned values */
        *modecount_return = 1;
        *modesinfo = modeinfoarr;
        
        return True;
    }

    RETURN_NATIVE(XF86VidModeGetAllModeLines, (dpy, screen, modecount_return, modesinfo), "libXxf86vm.so.1");
}

}
