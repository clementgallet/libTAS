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

#include "xinput.h"

#include "../hook.h"
#include "../logging.h"

namespace libtas {

int xinput_opcode;

DEFINE_ORIG_POINTER(XISelectEvents);
DEFINE_ORIG_POINTER(XIQueryDevice);

int XISelectEvents(Display* dpy, Window win, XIEventMask *masks, int num_masks)
{
    DEBUGLOGCALL(LCF_WINDOW);

    /* Only check if not using SDL */
    if (masks) {
        if (!(game_info.keyboard & (GameInfo::SDL1 | GameInfo::SDL2))) {
            if ((masks->mask_len >= XIMaskLen(XI_KeyPress)) && XIMaskIsSet(masks->mask, XI_KeyPress)) {
                debuglog(LCF_KEYBOARD, "   selecting XI keyboard events");
                game_info.keyboard |= GameInfo::XIEVENTS;
                game_info.tosend = true;
            }
            if ((masks->mask_len >= XIMaskLen(XI_RawKeyPress)) && XIMaskIsSet(masks->mask, XI_RawKeyPress)) {
                debuglog(LCF_KEYBOARD, "   selecting XI raw keyboard events");
                game_info.keyboard |= GameInfo::XIRAWEVENTS;
                game_info.tosend = true;
            }
        }

        if (!(game_info.mouse & (GameInfo::SDL1 | GameInfo::SDL2))) {
            if (((masks->mask_len >= XIMaskLen(XI_Motion)) && XIMaskIsSet(masks->mask, XI_Motion)) ||
                ((masks->mask_len >= XIMaskLen(XI_ButtonPress)) && XIMaskIsSet(masks->mask, XI_ButtonPress))) {
                debuglog(LCF_MOUSE, "   selecting XI mouse events");
                game_info.mouse |= GameInfo::XIEVENTS;
                game_info.tosend = true;
            }
            if (((masks->mask_len >= XIMaskLen(XI_RawMotion)) && XIMaskIsSet(masks->mask, XI_RawMotion)) ||
                ((masks->mask_len >= XIMaskLen(XI_RawButtonPress)) && XIMaskIsSet(masks->mask, XI_RawButtonPress))) {
                debuglog(LCF_MOUSE, "   selecting XI raw mouse events");
                game_info.mouse |= GameInfo::XIRAWEVENTS;
                game_info.tosend = true;
            }
        }
    }

    LINK_NAMESPACE(XISelectEvents, "Xi");
    return orig::XISelectEvents(dpy, win, masks, num_masks);
}

XIDeviceInfo* XIQueryDevice(Display* dpy, int deviceid, int* ndevices_return)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XIQueryDevice, "Xi");
    return orig::XIQueryDevice(dpy, deviceid, ndevices_return);
}

}
