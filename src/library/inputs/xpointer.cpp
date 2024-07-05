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

#include "xpointer.h"
#include "inputs.h"

#include "hook.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "xlib/XlibEventQueueList.h"
#include "xlib/xwindows.h" // x11::gameXWindows
#include "global.h"
#include "GlobalState.h"

namespace libtas {

Window pointer_grab_window = None;

DEFINE_ORIG_POINTER(XQueryPointer)

/* Override */ Bool XQueryPointer( Display* display, Window w,
        Window* root_return, Window* child_return,
        int* root_x_return, int* root_y_return,
        int* win_x_return, int* win_y_return,
        unsigned int* mask_return)
{
    LOGTRACE(LCF_MOUSE);
    if (x11::gameXWindows.empty()) {
        LINK_NAMESPACE_GLOBAL(XQueryPointer);
        return orig::XQueryPointer(display, w, root_return, child_return,
                                   root_x_return, root_y_return,
                                   win_x_return, win_y_return, mask_return);
    }
    *root_return = x11::rootWindow;
    *root_x_return = Inputs::game_ai.pointer.x;
    *root_y_return = Inputs::game_ai.pointer.y;
    *win_x_return = Inputs::game_ai.pointer.x;
    *win_y_return = Inputs::game_ai.pointer.y;
    *mask_return = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
    *child_return = 0;
    return 1;
}

/* Override */ int XGrabPointer(Display* display, Window w, Bool owner_events, unsigned int event_mask, int, int,
    Window confine_to, Cursor, Time)
{
    LOGTRACE(LCF_MOUSE);

    pointer_grab_window = w;
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    queue->grabPointer(pointer_grab_window, event_mask, owner_events);

    if (confine_to != None) {
        XWindowAttributes clip_attr;
        NATIVECALL(MYASSERT(XGetWindowAttributes(display, confine_to, &clip_attr) != 0));
        Inputs::pointer_clipping = true;
        Inputs::clipping_x = clip_attr.x;
        Inputs::clipping_y = clip_attr.y;
        Inputs::clipping_w = clip_attr.width;
        Inputs::clipping_h = clip_attr.height;

        if (Inputs::game_ai.pointer.x < Inputs::clipping_x) {
            LOG(LL_DEBUG, LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x);
            Inputs::game_ai.pointer.x = Inputs::clipping_x;
        }
        else if (Inputs::game_ai.pointer.x >= (Inputs::clipping_x + Inputs::clipping_w)) {
            LOG(LL_DEBUG, LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x + Inputs::clipping_w - 1);
            Inputs::game_ai.pointer.x = Inputs::clipping_x + Inputs::clipping_w - 1;
        }

        if (Inputs::game_ai.pointer.y < Inputs::clipping_y) {
            LOG(LL_DEBUG, LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y);
            Inputs::game_ai.pointer.y = Inputs::clipping_y;
        }
        else if (Inputs::game_ai.pointer.y >= (Inputs::clipping_y + Inputs::clipping_h)) {
            LOG(LL_DEBUG, LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y + Inputs::clipping_h - 1);
            Inputs::game_ai.pointer.y = Inputs::clipping_y + Inputs::clipping_h - 1;
        }
    }
    return GrabSuccess;
}

/* Override */ int XUngrabPointer(Display* display, Time)
{
    LOGTRACE(LCF_MOUSE);
    pointer_grab_window = None;
    
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    queue->ungrabPointer();

    Inputs::pointer_clipping = false;
    return 0; // Not sure what to return
}

/* Override */ int XChangeActivePointerGrab(Display*, unsigned int, Cursor, Time)
{
    LOGTRACE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XGrabButton(Display*, unsigned int, unsigned int, Window,
    Bool, unsigned int, int, int, Window, Cursor)
{
    LOGTRACE(LCF_MOUSE);
    return GrabSuccess;
}

/* Override */ int XUngrabButton(Display*, unsigned int, unsigned int, Window)
{
    LOGTRACE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XDefineCursor(Display* d, Window w, Cursor c)
{
    RETURN_IF_NATIVE(XDefineCursor, (d, w, c), nullptr);
    LOGTRACE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XUndefineCursor(Display* d, Window w)
{
    RETURN_IF_NATIVE(XUndefineCursor, (d, w), nullptr);
    LOGTRACE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XWarpPointer( Display* d, Window src_w, Window dest_w,
    int src_x, int src_y, unsigned int src_width, unsigned int src_height,
    int dest_x, int dest_y)
{
    RETURN_IF_NATIVE(XWarpPointer, (d, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y), nullptr);

    LOG(LL_TRACE, LCF_MOUSE, "%s called with dest_w %d and dest_x %d and dest_y %d", __func__, dest_w, dest_x, dest_y);

    /* We have to generate an MotionNotify event. */
    if (!x11::gameXWindows.empty()) {
        XEvent event;
        event.xmotion.type = MotionNotify;
        event.xmotion.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        if (dest_w == None) {
            /* Relative warp */
            event.xmotion.x = Inputs::game_ai.pointer.x + dest_x;
            event.xmotion.y = Inputs::game_ai.pointer.y + dest_y;
        }
        else {
            /* Absolute warp */
            event.xmotion.x = dest_x;
            event.xmotion.y = dest_y;
        }
        event.xmotion.x_root = event.xmotion.x;
        event.xmotion.y_root = event.xmotion.y;
        event.xmotion.window = x11::gameXWindows.front();

        struct timespec time = DeterministicTimer::get().getTicks();
        event.xmotion.time = time.tv_sec * 1000 + time.tv_nsec / 1000000;

        xlibEventQueueList.insert(&event);
        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event MotionNotify with new position (%d,%d)", Inputs::game_ai.pointer.x, Inputs::game_ai.pointer.y);
    }

    /* Update the pointer coordinates */
    if (dest_w == None) {
        /* Relative warp */
        Inputs::game_ai.pointer.x += dest_x;
        Inputs::game_ai.pointer.y += dest_y;
    }
    else {
        /* Absolute warp */
        Inputs::game_ai.pointer.x = dest_x;
        Inputs::game_ai.pointer.y = dest_y;
    }

    if (Global::shared_config.mouse_prevent_warp) {
        return 0; // Not sure what to return
    }
    
    /* When warping cursor, real and game cursor position are now synced.
     * When mouse is disabled, we consider that the user doesn't move the mouse,
     * so it is kept at the same position. */
    if (Global::shared_config.mouse_support) {
        if (dest_w == None) {
            /* Relative warp */
            Inputs::old_ai.pointer.x += dest_x;
            Inputs::old_ai.pointer.y += dest_y;
        }
        else {
            /* Absolute warp */
            Inputs::old_ai.pointer.x = dest_x;
            Inputs::old_ai.pointer.y = dest_y;
        }
    }

    RETURN_NATIVE(XWarpPointer, (d, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y), nullptr);
}

}
