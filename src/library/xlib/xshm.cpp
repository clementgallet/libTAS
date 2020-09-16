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

#include "xshm.h"
#ifdef LIBTAS_HAS_XRANDR

#include "../hook.h"
#include "../logging.h"
#include "../GlobalState.h"
#include "../../shared/SharedConfig.h"
#include "../ScreenCapture.h"
#include "../frame.h"
#include "../renderhud/RenderHUD_VDPAU.h"
#include "../DeterministicTimer.h"
// #include "../backtrace.h"
#include "../../shared/sockethelpers.h"
#include "../../shared/messages.h"

namespace libtas {

DEFINE_ORIG_POINTER(XShmPutImage);

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
    LINK_NAMESPACE(XShmPutImage, "XShm");

    if (GlobalState::isNative())
        return orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);

    debuglogstdio(LCF_WINDOW, "%s called with drawable %d", __func__, d);

    game_info.video |= GameInfo::XSHM;
    game_info.tosend = true;

    gameXImage = image;

    /* Set the game window to that window */
    if (gameXWindows.empty() || gameXWindows.front() != d) {
        /* Remove window from the list if it is already present */
        for (auto iter = gameXWindows.begin(); iter != gameXWindows.end(); iter++) {
            if (d == *iter) {
                gameXWindows.erase(iter);
                break;
            }
        }

        gameXWindows.push_front(d);
        uint32_t i = static_cast<uint32_t>(d);
        lockSocket();
        sendMessage(MSGB_WINDOW_ID);
        sendData(&i, sizeof(i));
        unlockSocket();
        debuglogstdio(LCF_WINDOW, "Sent X11 window id %d", i);
    }

    /* We must wait until the first screen draw to use the parameters of the
     * surface to initialize the screen capture.
     */
    ScreenCapture::init();

    /* The surface can change size independently of the window size, so we
     * must check here everytime. */

    /* Resize the screen capture. Only does something if size has changed */
    ScreenCapture::resize(image->width, image->height);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD renderHUD;
    frameBoundary([&] () {orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);}, renderHUD);
#else
    frameBoundary([&] () {orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);});
#endif

    return orig::XShmPutImage(dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);
}

}

#endif
