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

#include "ScreenCapture_SDL2_Surface.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "encoding/AVEncoder.h"
#include "GlobalState.h"
#include "sdl/sdlwindows.h" // sdl::gameSDLWindow

#include <SDL2/SDL.h>
#include <cstring> // memcpy

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GetWindowPixelFormat)
DECLARE_ORIG_POINTER(SDL_DestroyTexture)
DECLARE_ORIG_POINTER(SDL_GetWindowSurface)
DECLARE_ORIG_POINTER(SDL_ConvertSurfaceFormat)
DECLARE_ORIG_POINTER(SDL_FreeSurface)
DECLARE_ORIG_POINTER(SDL_SetClipRect)
DECLARE_ORIG_POINTER(SDL_GetClipRect)
DECLARE_ORIG_POINTER(SDL_LockSurface)
DECLARE_ORIG_POINTER(SDL_UnlockSurface)
DECLARE_ORIG_POINTER(SDL_UpperBlit)
DECLARE_ORIG_POINTER(SDL_GetWindowSize)

int ScreenCapture_SDL2_Surface::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    LINK_NAMESPACE_SDL2(SDL_GetWindowSize);
    orig::SDL_GetWindowSize(sdl::gameSDLWindow, &width, &height);
    LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
    Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl::gameSDLWindow);
    pixelSize = sdlpixfmt & 0xFF;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_SDL2_Surface::initScreenSurface()
{
    LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
    LINK_NAMESPACE_SDL2(SDL_ConvertSurfaceFormat);
    LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);

    SDL_Surface *surf = orig::SDL_GetWindowSurface(sdl::gameSDLWindow);
    screenSDL2Surf = orig::SDL_ConvertSurfaceFormat(surf, orig::SDL_GetWindowPixelFormat(sdl::gameSDLWindow), 0);
}

void ScreenCapture_SDL2_Surface::destroyScreenSurface()
{
    /* Delete the SDL2 screen surface */
    if (screenSDL2Surf) {
        LINK_NAMESPACE_SDL2(SDL_FreeSurface);
        orig::SDL_FreeSurface(screenSDL2Surf);
        screenSDL2Surf = nullptr;
    }

    /* Delete the SDL2 screen texture */
    if (screenSDLTex) {
        LINK_NAMESPACE_SDL2(SDL_DestroyTexture);
        orig::SDL_DestroyTexture(screenSDLTex);
        screenSDLTex = nullptr;
    }
}

const char* ScreenCapture_SDL2_Surface::getPixelFormat()
{
    LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
    Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl::gameSDLWindow);
    switch (sdlpixfmt) {
        case SDL_PIXELFORMAT_RGBA8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  RGBA");
            return "BGRA";
        case SDL_PIXELFORMAT_BGRA8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  BGRA");
            return "RGBA";
        case SDL_PIXELFORMAT_ARGB8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  ARGB");
            return "ABGR";
        case SDL_PIXELFORMAT_ABGR8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  ABGR");
            return "ARGB";
        case SDL_PIXELFORMAT_RGB888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  RGB888");
            return "BGR\0";
        case SDL_PIXELFORMAT_RGBX8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  RGBX8888");
            return "\0BGR";
        case SDL_PIXELFORMAT_BGR888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  BGR888");
            return "RGB\0";
        case SDL_PIXELFORMAT_BGRX8888:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  BGRX8888");
            return "\0RGB";
        case SDL_PIXELFORMAT_RGB24:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  RGB24");
            return "24BG";
        case SDL_PIXELFORMAT_BGR24:
            debuglogstdio(LCF_DUMP | LCF_SDL, "  BGR24");
            return "RAW ";
        default:
            debuglogstdio(LCF_DUMP | LCF_SDL | LCF_ERROR, "  Unsupported pixel format %d", sdlpixfmt);
    }

    return "RGBA";
}

int ScreenCapture_SDL2_Surface::copyScreenToSurface()
{
    GlobalNative gn;

    debuglogstdio(LCF_DUMP, "Access SDL_Surface pixels for video dump");

    LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
    LINK_NAMESPACE_SDL2(SDL_UpperBlit);

    /* Get surface from window */
    SDL_Surface* surf2 = orig::SDL_GetWindowSurface(sdl::gameSDLWindow);

    /* Checking for a size modification */
    int cw = surf2->w;
    int ch = surf2->h;
    if ((cw != width) || (ch != height)) {
        debuglogstdio(LCF_DUMP | LCF_ERROR, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, cw, ch);
        return -1;
    }

    orig::SDL_UpperBlit(surf2, nullptr, screenSDL2Surf, nullptr);
    
    return size;
}

int ScreenCapture_SDL2_Surface::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    // GlobalNative gn;

    LINK_NAMESPACE_SDL2(SDL_LockSurface);
    LINK_NAMESPACE_SDL2(SDL_UnlockSurface);
    /* We must lock the surface before accessing the raw pixels */
    if (SDL_MUSTLOCK(screenSDL2Surf))
        orig::SDL_LockSurface(screenSDL2Surf);

    /* I know memcpy is not recommended for vectors... */
    memcpy(winpixels.data(), screenSDL2Surf->pixels, size);

    /* Unlock surface */
    if (SDL_MUSTLOCK(screenSDL2Surf))
        orig::SDL_UnlockSurface(screenSDL2Surf);

    return size;
}

int ScreenCapture_SDL2_Surface::copySurfaceToScreen()
{
    GlobalNative gn;

    LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
    LINK_NAMESPACE_SDL2(SDL_UpperBlit);
    LINK_NAMESPACE_SDL2(SDL_GetClipRect);
    LINK_NAMESPACE_SDL2(SDL_SetClipRect);

    debuglogstdio(LCF_DUMP, "Set SDL1_Surface pixels");

    /* Get surface from window */
    SDL_Surface* surf2 = orig::SDL_GetWindowSurface(sdl::gameSDLWindow);

    /* Save and restore the clip rectangle */
    SDL_Rect clip_rect;
    orig::SDL_GetClipRect(surf2, &clip_rect);
    orig::SDL_SetClipRect(surf2, nullptr);
    orig::SDL_UpperBlit(screenSDL2Surf, nullptr, surf2, nullptr);
    orig::SDL_SetClipRect(surf2, &clip_rect);

    return 0;
}

void ScreenCapture_SDL2_Surface::restoreScreenState()
{
    copySurfaceToScreen();
}

}
