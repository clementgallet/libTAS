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

#include "xpointer.h"
#include "../hook.h"
#include "../logging.h"
#include "inputs.h"
#include "../../shared/AllInputs.h"
#include "../DeterministicTimer.h"
#include "../xlib/XlibEventQueueList.h"

namespace libtas {

DEFINE_ORIG_POINTER(XQueryPointer)

/* Override */ Bool XQueryPointer( Display* display, Window w,
        Window* root_return, Window* child_return,
        int* root_x_return, int* root_y_return,
        int* win_x_return, int* win_y_return,
        unsigned int* mask_return)
{
    DEBUGLOGCALL(LCF_MOUSE);
    if (gameXWindows.empty()) {
        LINK_NAMESPACE_GLOBAL(XQueryPointer);
        return orig::XQueryPointer(display, w, root_return, child_return,
                                   root_x_return, root_y_return,
                                   win_x_return, win_y_return, mask_return);
    }
    *root_return = rootWindow;
    *root_x_return = game_ai.pointer_x;
    *root_y_return = game_ai.pointer_y;
    *child_return = 0;
    *win_x_return = game_ai.pointer_x;
    *win_y_return = game_ai.pointer_y;
    *mask_return = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
    return 1;
}

/* Override */ int XGrabPointer(Display* display, Window w, Bool, unsigned int event_mask, int, int,
    Window confine_to, Cursor, Time)
{
    DEBUGLOGCALL(LCF_MOUSE);

    pointer_grab_window = w;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    queue->setMask(w, event_mask); // TODO: only set pointer event mask!!!

    if (confine_to != None) {
        XWindowAttributes clip_attr;
        NATIVECALL(MYASSERT(XGetWindowAttributes(display, confine_to, &clip_attr) != 0));
        pointer_clipping = true;
        clipping_x = clip_attr.x;
        clipping_y = clip_attr.y;
        clipping_w = clip_attr.width;
        clipping_h = clip_attr.height;

        if (game_ai.pointer_x < clipping_x) {
            debuglog(LCF_MOUSE, "   warping pointer x from ", game_ai.pointer_x, " to ", clipping_x);
            game_ai.pointer_x = clipping_x;
        }
        else if (game_ai.pointer_x >= (clipping_x + clipping_w)) {
            debuglog(LCF_MOUSE, "   warping pointer x from ", game_ai.pointer_x, " to ", clipping_x + clipping_w - 1);
            game_ai.pointer_x = clipping_x + clipping_w - 1;
        }

        if (game_ai.pointer_y < clipping_y) {
            debuglog(LCF_MOUSE, "   warping pointer y from ", game_ai.pointer_y, " to ", clipping_y);
            game_ai.pointer_y = clipping_y;
        }
        else if (game_ai.pointer_y >= (clipping_y + clipping_h)) {
            debuglog(LCF_MOUSE, "   warping pointer y from ", game_ai.pointer_y, " to ", clipping_y + clipping_h - 1);
            game_ai.pointer_y = clipping_y + clipping_h - 1;
        }
    }
    return GrabSuccess;
}

/* Override */ int XUngrabPointer(Display*, Time)
{
    DEBUGLOGCALL(LCF_MOUSE);
    pointer_grab_window = None;
    pointer_clipping = false;
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

/* Override */ int XDefineCursor(Display*, Window, Cursor)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XUndefineCursor(Display*, Window)
{
    DEBUGLOGCALL(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XWarpPointer( Display*, Window src_w, Window dest_w,
    int src_x, int src_y, unsigned int src_width, unsigned int src_height,
    int dest_x, int dest_y)
{
    debuglog(LCF_MOUSE, __func__, " called with dest_w ", dest_w, " and dest_x ", dest_x, " and dest_y ", dest_y);

    /* We have to generate an MotionNotify event. */
    if (!gameXWindows.empty()) {
        XEvent event;
        event.xmotion.type = MotionNotify;
        event.xmotion.state = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
        if (dest_w == None) {
            /* Relative warp */
            event.xmotion.x = game_ai.pointer_x + dest_x;
            event.xmotion.y = game_ai.pointer_y + dest_y;
        }
        else {
            /* Absolute warp */
            event.xmotion.x = dest_x;
            event.xmotion.y = dest_y;
        }
        event.xmotion.x_root = event.xmotion.x;
        event.xmotion.y_root = event.xmotion.y;
        event.xmotion.window = gameXWindows.front();

        struct timespec time = detTimer.getTicks();
        event.xmotion.time = time.tv_sec * 1000 + time.tv_nsec / 1000000;

        xlibEventQueueList.insert(&event);
        debuglog(LCF_EVENTS | LCF_MOUSE, "Generate Xlib event MotionNotify with new position (", game_ai.pointer_x, ",", game_ai.pointer_y,")");
    }

    /* Update the pointer coordinates */
    if (dest_w == None) {
        /* Relative warp */
        game_ai.pointer_x += dest_x;
        game_ai.pointer_y += dest_y;
    }
    else {
        /* Absolute warp */
        game_ai.pointer_x = dest_x;
        game_ai.pointer_y = dest_y;
    }

    return 0; // Not sure what to return
}



}
