/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ScreenCapture_VDPAU.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "encoding/AVEncoder.h"
#include "GlobalState.h"

namespace libtas {

DECLARE_ORIG_POINTER(VdpOutputSurfaceGetParameters)
DECLARE_ORIG_POINTER(VdpOutputSurfaceCreate)
DECLARE_ORIG_POINTER(VdpOutputSurfaceDestroy)
DECLARE_ORIG_POINTER(VdpOutputSurfaceRenderOutputSurface)
DECLARE_ORIG_POINTER(VdpOutputSurfaceGetBitsNative)

int ScreenCapture_VDPAU::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    /* Get window color depth */
    VdpRGBAFormat rgba_format;
    unsigned int uw, uh;
    orig::VdpOutputSurfaceGetParameters(vdp::vdpSurface, &rgba_format, &uw, &uh);
    if (rgba_format == VDP_RGBA_FORMAT_A8) {
        pixelSize = 1;
    }
    else {
        pixelSize = 4;
    }
    /* Also overwrite the dimensions */
    width = uw;
    height = uh;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_VDPAU::initScreenSurface()
{
    /* Set up a backup surface/framebuffer */
    VdpStatus status = orig::VdpOutputSurfaceCreate(vdp::vdpDevice, VDP_RGBA_FORMAT_B8G8R8A8, width, height, &screenVDPAUSurf);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceCreate failed with status %d", status);
        return;
    }
}

void ScreenCapture_VDPAU::destroyScreenSurface()
{
    if (screenVDPAUSurf) {
        orig::VdpOutputSurfaceDestroy(screenVDPAUSurf);
        screenVDPAUSurf = 0;
    }
}

const char* ScreenCapture_VDPAU::getPixelFormat()
{
    VdpRGBAFormat rgba_format;
    unsigned int uw, uh;
    orig::VdpOutputSurfaceGetParameters(vdp::vdpSurface, &rgba_format, &uw, &uh);
    switch (rgba_format) {
        case VDP_RGBA_FORMAT_B8G8R8A8:
            return "BGRA";
        case VDP_RGBA_FORMAT_R8G8B8A8:
            return "RGBA";
        default:
            debuglogstdio(LCF_DUMP | LCF_ERROR, "  Unsupported pixel format %d", rgba_format);
    }
    return "RGBA";
}

int ScreenCapture_VDPAU::copyScreenToSurface()
{
    GlobalNative gn;

    /* Copy to our screen surface */
    VdpStatus status = orig::VdpOutputSurfaceRenderOutputSurface(screenVDPAUSurf, nullptr, vdp::vdpSurface, nullptr, nullptr, nullptr, 0);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderOutputSurface failed with status %d", status);
    }
    
    return size;
}

int ScreenCapture_VDPAU::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;


    /* Copy pixels */
    void* const pix = reinterpret_cast<void* const>(winpixels.data());
    unsigned int pp = pitch;
    VdpStatus status = orig::VdpOutputSurfaceGetBitsNative(screenVDPAUSurf, nullptr, &pix, &pp);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceGetBitsNative failed with status %d", status);
    }

    return size;
}

int ScreenCapture_VDPAU::copySurfaceToScreen()
{
    GlobalNative gn;

    VdpStatus status = orig::VdpOutputSurfaceRenderOutputSurface(vdp::vdpSurface, nullptr, screenVDPAUSurf, nullptr, nullptr, nullptr, 0);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderOutputSurface failed with status %d", status);
    }

    return 0;
}

}
