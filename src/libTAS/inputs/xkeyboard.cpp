/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xkeyboard.h"
#include "../logging.h"
#include "inputs.h"
#include <X11/XKBlib.h>
#include <cstring> // memset
#include "../../shared/AllInputs.h"

namespace libtas {

int XQueryKeymap( Display* display, char keymap[32])
{
    DEBUGLOGCALL(LCF_KEYBOARD);

    memset(keymap, 0, 32);
    for (int kc=0; kc<256; kc++) {
        KeySym ks = XkbKeycodeToKeysym(display, (KeyCode)kc, 0, 0);
        for (int i=0; i<AllInputs::MAXKEYS; i++) {
            if (ks == ai.keyboard[i]) {
                keymap[kc>>3] |= (1 << kc&0x7);
                break;
            }
        }
    }

    /* I don't know what it should return */
    return 0;
}

/* Override */ Bool XQueryPointer( Display* display, Window w,
        Window* root_return, Window* child_return,
        int* root_x_return, int* root_y_return,
        int* win_x_return, int* win_y_return,
        unsigned int* mask_return)
{
    DEBUGLOGCALL(LCF_MOUSE);
    *win_x_return = ai.pointer_x;
    *win_y_return = ai.pointer_y;
    *mask_return = ai.pointer_mask;

    /* I don't know what it should return */
    return 1;
}

}
