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

#include "ScreenCapture_SDL1.h"
#include "hook.h"
#include "logging.h"
#include "../external/SDL1.h" // SDL_Surface
#include "global.h"
#include "GlobalState.h"

#include <cstring> // memcpy

namespace libtas {

/* Original function pointers */
namespace orig {
    static void (*SDL1_FreeSurface)(::SDL1::SDL_Surface *surface);
    static ::SDL1::SDL_Surface* (*SDL_GetVideoSurface)(void);
    static int (*SDL1_LockSurface)(::SDL1::SDL_Surface* surface);
    static void (*SDL1_UnlockSurface)(::SDL1::SDL_Surface* surface);
    static int (*SDL1_UpperBlit)(::SDL1::SDL_Surface *src, ::SDL1::SDL_Rect *srcrect, ::SDL1::SDL_Surface *dst, ::SDL1::SDL_Rect *dstrect);
    static uint8_t (*SDL1_SetClipRect)(::SDL1::SDL_Surface *surface, const ::SDL1::SDL_Rect *rect);
    static void (*SDL1_GetClipRect)(::SDL1::SDL_Surface *surface, ::SDL1::SDL_Rect *rect);
    static int (*SDL_SetAlpha)(::SDL1::SDL_Surface *surface, ::SDL1::Uint32 flag, ::SDL1::Uint8 alpha);
    static ::SDL1::SDL_Surface *(*SDL_DisplayFormat)(::SDL1::SDL_Surface *surface);
}

int ScreenCapture_SDL1::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    ::SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
    if (!surf) {
        return -1;
    }
    pixelSize = surf->format->BytesPerPixel;
    
    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_SDL1::initScreenSurface()
{
    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    LINK_NAMESPACE_SDL1(SDL_SetAlpha);
    LINK_NAMESPACE_SDL1(SDL_DisplayFormat);

    ::SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
    screenSDL1Surf = orig::SDL_DisplayFormat(surf);

    /* Disable alpha blending for that texture */
    if (screenSDL1Surf->flags & ::SDL1::SDL1_SRCALPHA) {
        orig::SDL_SetAlpha(screenSDL1Surf, 0, 0);
    }
}

void ScreenCapture_SDL1::destroyScreenSurface()
{
    /* Delete the SDL1 screen surface */
    if (screenSDL1Surf) {
        link_function((void**)&orig::SDL1_FreeSurface, "SDL_FreeSurface", "libSDL-1.2.so.0");
        orig::SDL1_FreeSurface(screenSDL1Surf);
        screenSDL1Surf = nullptr;
    }
}

const char* ScreenCapture_SDL1::getPixelFormat()
{
    switch (screenSDL1Surf->format->Rmask) {
        case 0x000000ff:
            return "RGBA";
        case 0x0000ff00:
            return "ARGB";
        case 0x00ff0000:
            return "BGRA";
        case 0xff000000:
            return "ABGR";
    }
    return "RGBA";
}

int ScreenCapture_SDL1::copyScreenToSurface()
{
    GlobalNative gn;

    /* Not tested !! */
    LOG(LL_DEBUG, LCF_DUMP, "Access SDL_Surface pixels for video dump");

    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    link_function((void**)&orig::SDL1_LockSurface, "SDL_LockSurface", "libSDL-1.2.so.0");
    link_function((void**)&orig::SDL1_UnlockSurface, "SDL_UnlockSurface", "libSDL-1.2.so.0");
    link_function((void**)&orig::SDL1_UpperBlit, "SDL_UpperBlit", "libSDL-1.2.so.0");
    LINK_NAMESPACE_SDL1(SDL_SetAlpha);

    /* Get surface from window */
    ::SDL1::SDL_Surface* surf1 = orig::SDL_GetVideoSurface();

    /* Checking for a size modification */
    int cw = surf1->w;
    int ch = surf1->h;
    if ((cw != width) || (ch != height)) {
        LOG(LL_ERROR, LCF_DUMP, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, cw, ch);
        return -1;
    }

    if (surf1->flags & ::SDL1::SDL1_SRCALPHA) {
        orig::SDL_SetAlpha(surf1, 0, 0);
        orig::SDL1_UpperBlit(surf1, nullptr, screenSDL1Surf, nullptr);
        orig::SDL_SetAlpha(surf1, ::SDL1::SDL1_SRCALPHA, 0);
    }
    else {
        orig::SDL1_UpperBlit(surf1, nullptr, screenSDL1Surf, nullptr);
    }

    return size;
}

int ScreenCapture_SDL1::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    /* We must lock the surface before accessing the raw pixels */
    int ret = orig::SDL1_LockSurface(screenSDL1Surf);
    if (ret != 0) {
        LOG(LL_ERROR, LCF_DUMP, "Could not lock SDL surface");
        return -1;
    }

    /* I know memcpy is not recommended for vectors... */
    memcpy(winpixels.data(), screenSDL1Surf->pixels, size);

    /* Unlock surface */
    orig::SDL1_UnlockSurface(screenSDL1Surf);

    return size;
}

int ScreenCapture_SDL1::copySurfaceToScreen()
{
    GlobalNative gn;

    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    link_function((void**)&orig::SDL1_GetClipRect, "SDL_GetClipRect", "libSDL-1.2.so.0");
    link_function((void**)&orig::SDL1_SetClipRect, "SDL_SetClipRect", "libSDL-1.2.so.0");
    link_function((void**)&orig::SDL1_UpperBlit, "SDL_UpperBlit", "libSDL-1.2.so.0");

    /* Not tested !! */
    LOG(LL_DEBUG, LCF_DUMP, "Set SDL1_Surface pixels");

    /* Get surface from window */
    ::SDL1::SDL_Surface* surf1 = orig::SDL_GetVideoSurface();

    /* Save and restore the clip rectangle */
    ::SDL1::SDL_Rect clip_rect;
    orig::SDL1_GetClipRect(surf1, &clip_rect);
    orig::SDL1_SetClipRect(surf1, nullptr);
    orig::SDL1_UpperBlit(screenSDL1Surf, nullptr, surf1, nullptr);
    orig::SDL1_SetClipRect(surf1, &clip_rect);

    return 0;
}

void ScreenCapture_SDL1::restoreScreenState()
{
    copySurfaceToScreen();
}

}
