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

#include "ScreenCapture_SDL2_Renderer.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "encoding/AVEncoder.h"
#include "GlobalState.h"
#include "sdl/sdlwindows.h" // sdl::gameSDLWindow

#include <SDL2/SDL.h>
#include <cstring> // memcpy


namespace libtas {

DECLARE_ORIG_POINTER(SDL_RenderReadPixels)
DECLARE_ORIG_POINTER(SDL_GetWindowPixelFormat)
DECLARE_ORIG_POINTER(SDL_GetRenderer)
DECLARE_ORIG_POINTER(SDL_CreateTexture)
DECLARE_ORIG_POINTER(SDL_LockTexture)
DECLARE_ORIG_POINTER(SDL_UnlockTexture)
DECLARE_ORIG_POINTER(SDL_RenderCopy)
DECLARE_ORIG_POINTER(SDL_DestroyTexture)
DECLARE_ORIG_POINTER(SDL_GetError)
DECLARE_ORIG_POINTER(SDL_GetWindowSize)
DECLARE_ORIG_POINTER(SDL_RenderClear)

int ScreenCapture_SDL2_Renderer::init()
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

void ScreenCapture_SDL2_Renderer::initScreenSurface()
{
    LINK_NAMESPACE_SDL2(SDL_GetRenderer);
    LINK_NAMESPACE_SDL2(SDL_CreateTexture);
    LINK_NAMESPACE_SDL2(SDL_GetError);
    LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);

    /* Create the screen texture */
    sdl_renderer = orig::SDL_GetRenderer(sdl::gameSDLWindow);
    if (!sdl_renderer) {
        debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_GetRenderer failed: %s", orig::SDL_GetError());
    }
    Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl::gameSDLWindow);
    if (!screenSDLTex) {
        screenSDLTex = orig::SDL_CreateTexture(sdl_renderer, sdlpixfmt,
            SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!screenSDLTex) {
            debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_CreateTexture failed: %s", orig::SDL_GetError());
        }
    }
}

void ScreenCapture_SDL2_Renderer::destroyScreenSurface()
{
    if (screenSDLTex) {
        LINK_NAMESPACE_SDL2(SDL_DestroyTexture);
        orig::SDL_DestroyTexture(screenSDLTex);
        screenSDLTex = nullptr;
    }
}

const char* ScreenCapture_SDL2_Renderer::getPixelFormat()
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

int ScreenCapture_SDL2_Renderer::copyScreenToSurface()
{
    GlobalNative gn;

    LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
    LINK_NAMESPACE_SDL2(SDL_LockTexture);
    LINK_NAMESPACE_SDL2(SDL_UnlockTexture);

    /* I couldn't find a way to directly copy the screen to a texture.
     * Because apparently there is no way to access to the screen texture.
     * So copying the screen pixels into the texture. */
    void* tex_pixels;
    int tex_pitch;
    int ret = orig::SDL_LockTexture(screenSDLTex, nullptr, &tex_pixels, &tex_pitch);
    if (ret < 0) {
        debuglogstdio(LCF_DUMP | LCF_SDL | LCF_ERROR, "SDL_LockTexture failed: %s", orig::SDL_GetError());
    }
    ret = orig::SDL_RenderReadPixels(sdl_renderer, NULL, 0, tex_pixels, tex_pitch);
    if (ret < 0) {
        debuglogstdio(LCF_DUMP | LCF_SDL | LCF_ERROR, "SDL_RenderReadPixels failed: %s", orig::SDL_GetError());
    }
    orig::SDL_UnlockTexture(screenSDLTex);

    return size;
}

int ScreenCapture_SDL2_Renderer::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    LINK_NAMESPACE_SDL2(SDL_LockTexture);
    LINK_NAMESPACE_SDL2(SDL_UnlockTexture);

    /* Access the texture and copy pixels */
    void* tex_pixels;
    int tex_pitch;
    orig::SDL_LockTexture(screenSDLTex, nullptr, &tex_pixels, &tex_pitch);
    memcpy(winpixels.data(), tex_pixels, size);
    orig::SDL_UnlockTexture(screenSDLTex);

    return size;
}

int ScreenCapture_SDL2_Renderer::copySurfaceToScreen()
{
    GlobalNative gn;

    LINK_NAMESPACE_SDL2(SDL_RenderCopy);

    int ret = orig::SDL_RenderCopy(sdl_renderer, screenSDLTex, NULL, NULL);
    if (ret < 0) {
        debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_RenderCopy to screen failed: %s", orig::SDL_GetError());
    }

    return 0;
}

void ScreenCapture_SDL2_Renderer::restoreScreenState()
{
    copySurfaceToScreen();
}

void ScreenCapture_SDL2_Renderer::clearScreen()
{
    LINK_NAMESPACE_SDL2(SDL_RenderClear);
    orig::SDL_RenderClear(sdl_renderer);
}

uint64_t ScreenCapture_SDL2_Renderer::screenTexture()
{
    return reinterpret_cast<uint64_t>(screenSDLTex);
}

}
