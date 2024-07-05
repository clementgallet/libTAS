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

#include "ScreenCapture_Impl.h"
#include "ScreenCapture.h"

#include "hook.h"
#include "logging.h"
#include "global.h"
#include "encoding/AVEncoder.h"
#include "GlobalState.h"
#include "xlib/xwindows.h"
#include "xlib/xdisplay.h"

#include <X11/Xlib.h>
// #include <SDL2/SDL.h>

namespace libtas {

#ifdef __unix__
DEFINE_ORIG_POINTER(XGetGeometry)
#endif

int ScreenCapture_Impl::init()
{
#ifdef __unix__
    /* Don't initialize if window is not registered */
    if (x11::gameXWindows.empty())
        return -1;
#else
#error "ScreenCapture not implemented for MacOS"
    // /* Use SDL window for now */
    // if ((Global::game_info.video & GameInfo::SDL2) && (!sdl::gameSDLWindow))
    //     return 0;
#endif

    unsigned int depth = 8;

    /* XGetGeometry() may not be synced to the last dimensions, to we look
     * first if a resize was performed before init */
    if (ScreenCapture::width != 0) {
        width = ScreenCapture::width;
        height = ScreenCapture::height;
    }
    else {
#ifdef __unix__
        /* Get the window dimensions */
        LINK_NAMESPACE_GLOBAL(XGetGeometry);
        int x, y;
        unsigned int w = 0, h = 0, border_width;
        Window root;
        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (x11::gameDisplays[i]) {
                orig::XGetGeometry(x11::gameDisplays[i], x11::gameXWindows.front(), &root, &x, &y, &w, &h, &border_width, &depth);
                break;
            }
        }
        width = w;
        height = h;
#else
#error "ScreenCapture not implemented for MacOS"
        /* Use SDL2 window for now */
        // if (Global::game_info.video & GameInfo::SDL2) {
            //     LINK_NAMESPACE_SDL2(SDL_GetWindowSize);
            //     orig::SDL_GetWindowSize(sdl::gameSDLWindow, &width, &height);
            // }
#endif
    }

    return 0;
}

int ScreenCapture_Impl::postInit()
{
    size = width * height * pixelSize;
    pitch = pixelSize * width;

    winpixels.resize(size);

    initScreenSurface();

    LOG(LL_DEBUG, LCF_WINDOW, "Inited Screen Capture with dimensions (%d,%d)", width, height);
    return 0;    
}

void ScreenCapture_Impl::fini()
{
    winpixels.clear();

    destroyScreenSurface();
}

void ScreenCapture_Impl::resize(int w, int h)
{
#ifdef __unix__
    /* Don't resize if window is not registered */
    if (x11::gameXWindows.empty()) {
        return;
    }
#endif

    destroyScreenSurface();

    width = w;
    height = h;
    size = width * height * pixelSize;
    pitch = pixelSize * width;

    winpixels.resize(size);

    initScreenSurface();

    /* We need to close the dumping if needed, and open a new one */
    if (Global::shared_config.av_dumping) {
        avencoder.reset(new AVEncoder());
    }

    LOG(LL_DEBUG, LCF_WINDOW, "Resize Screen Capture with new dimensions (%d,%d) and size %d", width, height, size);
}

void ScreenCapture_Impl::getDimensions(int& w, int& h) {
    w = width;
    h = height;
}

int ScreenCapture_Impl::getSize()
{
    return size;
}

}
