/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "videocapture.h"

#include "hook.h"
#include "logging.h"
#include "../external/gl.h" // glReadPixels enum arguments
#include "../external/SDL.h" // SDL_Surface
#include <vector>
#include <string.h> // memcpy

bool useGL;

/* Temporary pixel arrays */
std::vector<uint8_t> glpixels;
std::vector<uint8_t> winpixels;

/* Original function pointers */
namespace orig {
    static void (*SDL_GL_GetDrawableSize)(void* window, int* w, int* h);
    static SDL1::SDL_Surface* (*SDL_GetVideoSurface)(void);
    static int (*SDL_LockSurface)(SDL1::SDL_Surface* surface);
    static void (*SDL_UnlockSurface)(SDL1::SDL_Surface* surface);
    static int (*SDL_RenderReadPixels)(void*, const SDL_Rect*, Uint32, void*, int);
    static Uint32 (*SDL_GetWindowPixelFormat)(void* window);
    static void* (*SDL_GetRenderer)(void* window);
    static int (*SDL_GetRendererOutputSize)(void* renderer, int* w, int* h);
    static void (*glReadPixels)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);
}

/* Video dimensions */
int width, height;
int size;

void* renderer;
int pixelSize = 0;

int initVideoCapture(void* window, bool video_opengl, int *pwidth, int *pheight)
{
    /* If the game uses openGL, the method to capture the screen will be different */
    useGL = video_opengl;

    /* Link the required functions, and get the window dimensions */
    if (SDLver == 1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        if (useGL)
            LINK_NAMESPACE(glReadPixels, "libGL");
        else {
            LINK_NAMESPACE_SDL1(SDL_LockSurface);
            LINK_NAMESPACE_SDL1(SDL_UnlockSurface);
        }

        /* Get dimensions from the window surface */
        if (!orig::SDL_GetVideoSurface) {
            debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Need function SDL_GetVideoSurface.");
            return -1;
        }

        SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();

        pixelSize = surf->format->BytesPerPixel;

        *pwidth = surf->w;
        *pheight = surf->h;
    }
    else if (SDLver == 2) {

        if (useGL) {
            LINK_NAMESPACE_SDL2(SDL_GL_GetDrawableSize);
            LINK_NAMESPACE(glReadPixels, "libGL");

            /* Get information about the current screen */
            if (!orig::SDL_GL_GetDrawableSize) {
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Need function SDL_GL_GetDrawableSize.");
                return -1;
            }

            orig::SDL_GL_GetDrawableSize(window, pwidth, pheight);
            pixelSize = 4;
        }
        else {
            LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
            LINK_NAMESPACE_SDL2(SDL_GetRenderer);
            LINK_NAMESPACE_SDL2(SDL_GetRendererOutputSize);
            LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);

            Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(window);
            pixelSize = sdlpixfmt & 0xFF;

            /* Get surface from window */
            if (!orig::SDL_GetRendererOutputSize) {
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Need function SDL_GetWindowSize.");
                return -1;
            }

            renderer = orig::SDL_GetRenderer(window);
            orig::SDL_GetRendererOutputSize(renderer, pwidth, pheight);
        }
    }
    else {
        debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Unknown SDL version.");
        return -1;
    }

    /* Save dimensions for later */
    width = *pwidth;
    height = *pheight;

    /* Allocate an array of pixels */
    size = width * height * pixelSize;
    winpixels.resize(size);

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglog(LCF_DUMP | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return -1;
    }

    if (useGL) {
        /* Do we already have access to the glReadPixels function? */
        if (!orig::glReadPixels) {
            debuglog(LCF_DUMP | LCF_OGL | LCF_ERROR, "Could not load function glReadPixels.");
            return -1;
        }

        /* Allocate another pixels array,
         * because the image will need to be flipped.
         */
        glpixels.resize(size);
    }

    return 0;
}

#ifdef LIBTAS_ENABLE_AVDUMPING

AVPixelFormat getPixelFormat(void* window)
{
    if (useGL) {
        return AV_PIX_FMT_RGBA;
    }

    if (SDLver == 1) {
        // SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
        debuglog(LCF_DUMP | LCF_SDL | LCF_TODO, "We assumed pixel format is RGBA");
        return AV_PIX_FMT_RGBA; // TODO
    }
    else if (SDLver == 2) {
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(window);
        /* TODO: There are probably helper functions to build pixel
         * format constants from channel masks
         */
        switch (sdlpixfmt) {
            case SDL_PIXELFORMAT_RGBA8888:
                debuglog(LCF_DUMP | LCF_SDL, "  RGBA");
                return AV_PIX_FMT_BGRA;
            case SDL_PIXELFORMAT_BGRA8888:
                debuglog(LCF_DUMP | LCF_SDL, "  BGRA");
                return AV_PIX_FMT_RGBA;
            case SDL_PIXELFORMAT_ARGB8888:
                debuglog(LCF_DUMP | LCF_SDL, "  ARGB");
                return AV_PIX_FMT_ABGR;
            case SDL_PIXELFORMAT_ABGR8888:
                debuglog(LCF_DUMP | LCF_SDL, "  ABGR");
                return AV_PIX_FMT_ARGB;
            case SDL_PIXELFORMAT_RGB24:
                debuglog(LCF_DUMP | LCF_SDL, "  RGB24");
                return AV_PIX_FMT_BGR24;
            case SDL_PIXELFORMAT_BGR24:
                debuglog(LCF_DUMP | LCF_SDL, "  BGR24");
                return AV_PIX_FMT_RGB24;
            case SDL_PIXELFORMAT_RGB888:
                debuglog(LCF_DUMP | LCF_SDL, "  RGB888");
                return AV_PIX_FMT_BGR0;
            case SDL_PIXELFORMAT_RGBX8888:
                debuglog(LCF_DUMP | LCF_SDL, "  RGBX888");
                return AV_PIX_FMT_RGB0;
            case SDL_PIXELFORMAT_BGR888:
                debuglog(LCF_DUMP | LCF_SDL, "  RGBX888");
                return AV_PIX_FMT_0BGR;
            case SDL_PIXELFORMAT_BGRX8888:
                return AV_PIX_FMT_BGR0;
            default:
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "  Unsupported pixel format");
        }
    }

    return AV_PIX_FMT_NONE;
}

#endif

int captureVideoFrame(const uint8_t* orig_plane[], int orig_stride[])
{
    int pitch = pixelSize * width;

    if (useGL) {
        /* TODO: Check that the openGL dimensions did not change in between */

        /* We access to the image pixels directly using glReadPixels */
        orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &glpixels[0]);
        /* TODO: I saw this in some examples before calling glReadPixels: glPixelStorei(GL_PACK_ALIGNMENT, 1); */

        /*
         * Flip image horizontally
         * This is because OpenGL has a different reference point
         * Code taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
         * TODO: Could this be done without allocating another array ?
         */

        for (int line = 0; line < height; line++) {
            int pos = line * pitch;
            for (int p=0; p<pitch; p++) {
                winpixels[pos + p] = glpixels[(size-pos)-pitch + p];
            }
        }

    }

    else {
        /* Not tested !! */
        debuglog(LCF_DUMP | LCF_UNTESTED | LCF_FRAME, "Access SDL_Surface pixels for video dump");

        if (SDLver == 1) {
            /* Get surface from window */
            SDL1::SDL_Surface* surf1 = orig::SDL_GetVideoSurface();

            /* Checking for a size modification */
            int cw = surf1->w;
            int ch = surf1->h;
            if ((cw != width) || (ch != height)) {
                debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",cw,",",ch,")");
                return -1;
            }

            /* We must lock the surface before accessing the raw pixels */
            int ret = orig::SDL_LockSurface(surf1);
            if (ret != 0) {
                debuglog(LCF_DUMP | LCF_ERROR, "Could not lock SDL surface");
                return -1;
            }

            /* I know memcpy is not recommended for vectors... */
            memcpy(winpixels.data(), surf1->pixels, size);

            /* Unlock surface */
            orig::SDL_UnlockSurface(surf1);
        }

        if (SDLver == 2) {
            /* Checking for a size modification */
            int cw, ch;
            orig::SDL_GetRendererOutputSize(renderer, &cw, &ch);
            if ((cw != width) || (ch != height)) {
                debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",cw,",",ch,")");
                return -1;
            }

            orig::SDL_RenderReadPixels(renderer, NULL, 0, winpixels.data(), pitch);
        }
    }

    orig_plane[0] = const_cast<const uint8_t*>(winpixels.data());
    orig_stride[0] = pitch;

    return 0;
}
