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

#include "ScreenCapture_SDL3_Renderer.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "encoding/AVEncoder.h"
#include "GlobalState.h"
#include "sdl/sdlwindows.h" // sdl::gameSDLWindow
#include "sdl/sdldynapi.h"

#include "../external/SDL3.h"
#include <cstring> // memcpy


namespace libtas {

int ScreenCapture_SDL3_Renderer::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    ORIG_SDL3_CALL(SDL_GetWindowSize, (sdl::gameSDLWindow, &width, &height));
    sdl3::SDL_PixelFormat sdlpixfmt = ORIG_SDL3_CALL(SDL_GetWindowPixelFormat, (sdl::gameSDLWindow));
    pixelSize = sdlpixfmt & 0xFF;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_SDL3_Renderer::initScreenSurface()
{
    /* Create the screen texture */
    sdl_renderer = ORIG_SDL3_CALL(SDL_GetRenderer, (sdl::gameSDLWindow));
    if (!sdl_renderer) {
        LOG(LL_ERROR, LCF_WINDOW | LCF_SDL, "SDL_GetRenderer failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
    }
    sdl3::SDL_PixelFormat sdlpixfmt = ORIG_SDL3_CALL(SDL_GetWindowPixelFormat, (sdl::gameSDLWindow));
    if (!screenSDLTex) {
        screenSDLTex = ORIG_SDL3_CALL(SDL_CreateTexture, (sdl_renderer, sdlpixfmt,
            sdl3::SDL_TEXTUREACCESS_STREAMING, width, height));
        if (!screenSDLTex) {
            LOG(LL_ERROR, LCF_WINDOW | LCF_SDL, "SDL_CreateTexture failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
        }
    }
}

void ScreenCapture_SDL3_Renderer::destroyScreenSurface()
{
    if (screenSDLTex) {
        ORIG_SDL3_CALL(SDL_DestroyTexture, (screenSDLTex));
        screenSDLTex = nullptr;
    }
}

const char* ScreenCapture_SDL3_Renderer::getPixelFormat()
{
    sdl3::SDL_PixelFormat sdlpixfmt = ORIG_SDL3_CALL(SDL_GetWindowPixelFormat, (sdl::gameSDLWindow));
    switch (sdlpixfmt) {
        case sdl3::SDL_PIXELFORMAT_RGBA8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  RGBA");
            return "BGRA";
        case sdl3::SDL_PIXELFORMAT_BGRA8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  BGRA");
            return "RGBA";
        case sdl3::SDL_PIXELFORMAT_ARGB8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  ARGB");
            return "ABGR";
        case sdl3::SDL_PIXELFORMAT_ABGR8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  ABGR");
            return "ARGB";
        case sdl3::SDL_PIXELFORMAT_XRGB8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  RGB888");
            return "BGR\0";
        case sdl3::SDL_PIXELFORMAT_RGBX8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  RGBX8888");
            return "\0BGR";
        case sdl3::SDL_PIXELFORMAT_XBGR8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  BGR888");
            return "RGB\0";
        case sdl3::SDL_PIXELFORMAT_BGRX8888:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  BGRX8888");
            return "\0RGB";
        case sdl3::SDL_PIXELFORMAT_RGB24:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  RGB24");
            return "24BG";
        case sdl3::SDL_PIXELFORMAT_BGR24:
            LOG(LL_DEBUG, LCF_DUMP | LCF_SDL, "  BGR24");
            return "RAW ";
        default:
            LOG(LL_ERROR, LCF_DUMP | LCF_SDL, "  Unsupported pixel format %d", sdlpixfmt);
    }

    return "RGBA";
}

int ScreenCapture_SDL3_Renderer::copyScreenToSurface()
{
    GlobalNative gn;

    /* I couldn't find a way to directly copy the screen to a texture.
     * Because apparently there is no way to access to the screen texture.
     * So copying the screen pixels into the texture. */
    sdl3::SDL_Surface* texSurface;
    bool ret = ORIG_SDL3_CALL(SDL_LockTextureToSurface, (screenSDLTex, nullptr, &texSurface));
    if (!ret) {
        LOG(LL_ERROR, LCF_DUMP | LCF_SDL, "SDL_LockTextureToSurface failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
    }
    sdl3::SDL_Surface* screenSurface = ORIG_SDL3_CALL(SDL_RenderReadPixels, (sdl_renderer, NULL));

    ret = ORIG_SDL3_CALL(SDL_BlitSurface, (screenSurface, NULL, texSurface, NULL));
    if (!ret) {
        LOG(LL_ERROR, LCF_DUMP | LCF_SDL, "SDL_BlitSurface failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
    }

    ORIG_SDL3_CALL(SDL_DestroySurface, (screenSurface));
    ORIG_SDL3_CALL(SDL_UnlockTexture, (screenSDLTex));

    return size;
}

int ScreenCapture_SDL3_Renderer::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    /* Access the texture and copy pixels */
    void* tex_pixels;
    int tex_pitch;
    int ret = ORIG_SDL3_CALL(SDL_LockTexture, (screenSDLTex, nullptr, &tex_pixels, &tex_pitch));
    if (!ret) {
        LOG(LL_ERROR, LCF_DUMP | LCF_SDL, "SDL_LockTexture failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
        return -1;
    }

    ret = copyPixelRows(tex_pixels, tex_pitch);
    ORIG_SDL3_CALL(SDL_UnlockTexture, (screenSDLTex));

    return ret;
}

int ScreenCapture_SDL3_Renderer::copySurfaceToScreen()
{
    GlobalNative gn;

    bool ret = ORIG_SDL3_CALL(SDL_RenderTexture, (sdl_renderer, screenSDLTex, NULL, NULL));
    if (!ret) {
        LOG(LL_ERROR, LCF_WINDOW | LCF_SDL, "SDL_RenderTexture to screen failed: %s", ORIG_SDL3_CALL(SDL_GetError, ()));
    }

    return 0;
}

void ScreenCapture_SDL3_Renderer::restoreScreenState()
{
    copySurfaceToScreen();
}

void ScreenCapture_SDL3_Renderer::clearScreen()
{
    ORIG_SDL3_CALL(SDL_RenderClear, (sdl_renderer));
}

uint64_t ScreenCapture_SDL3_Renderer::screenTexture()
{
    return reinterpret_cast<uint64_t>(screenSDLTex);
}

}
