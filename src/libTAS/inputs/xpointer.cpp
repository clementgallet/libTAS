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

#include "xpointer.h"
#include "../logging.h"
#include "inputs.h"
#include "../../shared/AllInputs.h"

namespace libtas {

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

/* Override */ int XGrabPointer(Display*, Window, Bool, unsigned int, int, int,
    Window, Cursor, Time)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return GrabSuccess;
}

/* Override */ int XUngrabPointer(Display*, Time)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XChangeActivePointerGrab(Display*, unsigned int, Cursor, Time)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XGrabButton(Display*, unsigned int, unsigned int, Window,
    Bool, unsigned int, int, int, Window, Cursor)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return GrabSuccess;
}

/* Override */ int XUngrabButton(Display*, unsigned int, unsigned int, Window)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return 0; // Not sure what to return
}


}
