/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ScreenCapture.h"
#include "ScreenCapture_GL.h"
#include "ScreenCapture_SDL1.h"
#include "ScreenCapture_SDL2_Renderer.h"
#include "ScreenCapture_SDL3_Renderer.h"
#include "ScreenCapture_SDL2_Surface.h"
#include "ScreenCapture_VDPAU.h"
#include "ScreenCapture_Vulkan.h"
#include "ScreenCapture_XShm.h"
#include "logging.h"
#include "global.h"

namespace libtas {

ScreenCapture_Impl* ScreenCapture::impl = nullptr;
bool ScreenCapture::inited = false;
int ScreenCapture::width = 0;
int ScreenCapture::height = 0;

int ScreenCapture::init()
{
    if (inited) {
        return -1;
    }
    
    if (!impl) {
        if (Global::game_info.video & GameInfo::VDPAU) {
            impl = new ScreenCapture_VDPAU();
        }
        else if (Global::game_info.video & GameInfo::XSHM) {
            impl = new ScreenCapture_XShm();
        }
        else if (Global::game_info.video & GameInfo::SDL3_RENDERER) {
            impl = new ScreenCapture_SDL3_Renderer();
        }    
        else if (Global::game_info.video & GameInfo::SDL2_RENDERER) {
            impl = new ScreenCapture_SDL2_Renderer();
        }    
        else if (Global::game_info.video & GameInfo::SDL2_SURFACE) {
            impl = new ScreenCapture_SDL2_Surface();
        }
        else if (Global::game_info.video & GameInfo::OPENGL) {
            impl = new ScreenCapture_GL();
        }
        else if (Global::game_info.video & GameInfo::SDL1) {
            impl = new ScreenCapture_SDL1();
        }
        else if (Global::game_info.video & GameInfo::VULKAN) {
            impl = new ScreenCapture_Vulkan();
        }
    }
    
    if (impl) {
        int ret = impl->init();
        if (ret < 0) return ret;
        inited = true;
        return 0;
    }
    return -1;
}

void ScreenCapture::fini()
{
    width = 0;
    height = 0;
    
    if (!inited) return;

    inited = false;

    if (impl) {
        impl->fini();
        delete impl;
        impl = nullptr;
    }
}

void ScreenCapture::resize(int w, int h)
{
    if (!inited) {
        width = w;
        height = h;
        return;
    }

    if (impl) {
        impl->resize(w, h);
    }
}

bool ScreenCapture::isInited()
{
    return inited;
}

void ScreenCapture::getDimensions(int& w, int& h) {
    if (impl) {
        impl->getDimensions(w, h);
    }
    else if (width != 0 && height != 0) {
        w = width;
        h = height;
    }
}

int ScreenCapture::getSize()
{
    if (impl) {
        return impl->getSize();
    }
    return 0;
}

int ScreenCapture::getPixelFormat()
{
    MYASSERT(inited)

    if (impl) {
        return impl->getPixelFormat();
    }
    return PIXELFORMAT_UNKNOWN;
}

const char* ScreenCapture::pixelFormatToFourCC(int pixel_format)
{
    switch(pixel_format) {
        case PIXELFORMAT_RGBA8:
            return "RGBA";
        case PIXELFORMAT_BGRA8:
            return "BGRA";
        case PIXELFORMAT_ARGB8:
            return "ARGB";
        case PIXELFORMAT_ABGR8:
            return "ABGR";
        case PIXELFORMAT_RGBX8:
            return "RGB\0";
        case PIXELFORMAT_BGRX8:
            return "BGR\0";
        case PIXELFORMAT_XRGB8:
            return "\0RGB";
        case PIXELFORMAT_XBGR8:
            return "\0BGR";
        case PIXELFORMAT_RGB24:
            return "RAW ";
        case PIXELFORMAT_BGR24:
            return "24BG";
        case PIXELFORMAT_RGBA16:
            return "RBA\x40";
        default:
            LOG(LL_ERROR, LCF_DUMP, "  Unsupported pixel format %d", pixel_format);
    }
    return "";
}

int ScreenCapture::getPixelFormatDepth(int pixel_format)
{
    switch(pixel_format) {
        case PIXELFORMAT_RGBA8:
        case PIXELFORMAT_BGRA8:
        case PIXELFORMAT_ARGB8:
        case PIXELFORMAT_ABGR8:
        case PIXELFORMAT_RGBX8:
        case PIXELFORMAT_BGRX8:
        case PIXELFORMAT_XRGB8:
        case PIXELFORMAT_XBGR8:
            return 4;
        case PIXELFORMAT_RGB24:
        case PIXELFORMAT_BGR24:
            return 3;
        case PIXELFORMAT_RGBA16:
            return 8;
        default:
            LOG(LL_ERROR, LCF_DUMP, "  Unsupported pixel format %d", pixel_format);
            return 0;
    }
}

int ScreenCapture::copyScreenToSurface()
{
    if (!inited)
        return 0;

    if (impl) {
        return impl->copyScreenToSurface();
    }
    return 0;
}

int ScreenCapture::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (!inited)
        return 0;

    if (impl) {
        return impl->getPixelsFromSurface(pixels, draw);
    }
    return 0;
}

int ScreenCapture::copySurfaceToScreen()
{
    if (!inited)
        return 0;

    if (impl) {
        return impl->copySurfaceToScreen();
    }
    return 0;
}

void ScreenCapture::restoreScreenState()
{
    if (!inited)
        return;

    if (impl) {
        impl->restoreScreenState();
    }
}

void ScreenCapture::clearScreen()
{
    if (!inited)
        return;

    if (impl) {
        impl->clearScreen();
    }
}

uint64_t ScreenCapture::screenTexture()
{
    if (!inited)
        return 0;

    if (impl) {
        return impl->screenTexture();
    }
    return 0;
}

}
