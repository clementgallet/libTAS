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

#include "ScreenCapture.h"
#include "ScreenCapture_GL.h"
#include "ScreenCapture_SDL1.h"
#include "ScreenCapture_SDL2_Renderer.h"
#include "ScreenCapture_SDL2_Surface.h"
#include "ScreenCapture_VDPAU.h"
#include "ScreenCapture_Vulkan.h"
#include "ScreenCapture_XShm.h"
#include "logging.h"
#include "global.h"

namespace libtas {

ScreenCapture_Impl* ScreenCapture::impl = nullptr;
bool ScreenCapture::inited = false;

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
        impl->init();
        inited = true;
    }
    
    return 0;
}

void ScreenCapture::fini()
{
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
}

int ScreenCapture::getSize()
{
    if (impl) {
        return impl->getSize();
    }
    return 0;
}

const char* ScreenCapture::getPixelFormat()
{
    MYASSERT(inited)

    if (impl) {
        return impl->getPixelFormat();
    }
    return "";
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
        return impl->clearScreen();
    }
}

uint32_t ScreenCapture::screenTexture()
{
    if (!inited)
        return 0;

    if (impl) {
        return impl->screenTexture();
    }
    return 0;
}

}
