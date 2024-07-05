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

#include "xshm.h"
#include "xwindows.h" // x11::gameXWindows

#include "hook.h"
#include "logging.h"
#include "GlobalState.h"
#include "screencapture/ScreenCapture.h"
#include "frame.h"
#include "renderhud/RenderHUD.h"
#include "DeterministicTimer.h"
#include "global.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

namespace libtas {

DEFINE_ORIG_POINTER(XShmPutImage)

XImage* x11::gameXImage = nullptr;

OVERRIDE Bool XShmPutImage(
    Display*        dpy,
    Drawable        d,
    GC          gc,
    XImage*     image,
    int         src_x,
    int         src_y,
    int         dst_x,
    int         dst_y,
    unsigned int    src_width,
    unsigned int    src_height,
    Bool        send_event
)
{
    LINK_NAMESPACE_FULLNAME(XShmPutImage, "libXext.so.6");

    if (GlobalState::isNative())
        return orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with drawable %d", __func__, d);

    /* Only handle the screen draw if no other rendering API is used */
    if ((Global::game_info.video & GameInfo::VDPAU) || (Global::game_info.video & GameInfo::OPENGL))
        return orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);

    Global::game_info.video |= GameInfo::XSHM;
    Global::game_info.tosend = true;

    x11::gameXImage = image;

    /* Set the game window to that window */
    if (x11::gameXWindows.empty() || x11::gameXWindows.front() != d) {
        /* Remove window from the list if it is already present */
        for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
            if (d == *iter) {
                x11::gameXWindows.erase(iter);
                break;
            }
        }

        x11::gameXWindows.push_front(d);
        uint32_t i = static_cast<uint32_t>(d);
        lockSocket();
        sendMessage(MSGB_WINDOW_ID);
        sendData(&i, sizeof(i));
        unlockSocket();
        LOG(LL_DEBUG, LCF_WINDOW, "Sent X11 window id %d", i);
    }

    /* The surface can change size independently of the window size, so we
     * must check here everytime. */

    /* Resize the screen capture. Only does something if size has changed */
    ScreenCapture::resize(image->width, image->height);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD renderHUD;
    frameBoundary([&] () {orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);}, renderHUD);

    return True;
    // return orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);
}

}
