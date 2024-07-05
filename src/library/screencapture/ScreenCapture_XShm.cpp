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

#include "ScreenCapture_XShm.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#include "xlib/xshm.h" // x11::gameXImage

#include <cstring> // memcpy

namespace libtas {

int ScreenCapture_XShm::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;
    
    pixelSize = x11::gameXImage->bits_per_pixel / 8;
    /* Also overwrite the dimensions */
    width = x11::gameXImage->width;
    height = x11::gameXImage->height;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_XShm::initScreenSurface() {}

void ScreenCapture_XShm::destroyScreenSurface() {}

const char* ScreenCapture_XShm::getPixelFormat()
{
    /* Apparently, it will only be RGB or BGR depending on the endianness
     * of the machine. */

    if (x11::gameXImage->bits_per_pixel == 24) {
        if (x11::gameXImage->byte_order == LSBFirst)
            return "24BG";
        else
            return "RAW ";
    }
    else if (x11::gameXImage->bits_per_pixel == 32) {
        if (x11::gameXImage->byte_order == LSBFirst)
            return "BGR\0";
        else
            return "RGB\0";
    }
    else {
        LOG(LL_ERROR, LCF_DUMP, "  Unsupported pixel format");
    }

    return "RGBA";
}

int ScreenCapture_XShm::copyScreenToSurface()
{
    GlobalNative gn;

    if ((x11::gameXImage->width != width) || (x11::gameXImage->height != height)) {
        LOG(LL_ERROR, LCF_DUMP, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, x11::gameXImage->width, x11::gameXImage->height);
        return -1;
    }
    
    /* There is no designated surface for XShm, just copy to our array */
    memcpy(winpixels.data(), x11::gameXImage->data, size);
    
    return size;
}

int ScreenCapture_XShm::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    return size;
}

int ScreenCapture_XShm::copySurfaceToScreen()
{
    memcpy(x11::gameXImage->data, winpixels.data(), size);

    return 0;
}

void ScreenCapture_XShm::restoreScreenState()
{
    copySurfaceToScreen();
}

}
