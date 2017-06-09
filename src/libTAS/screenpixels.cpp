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

#include "screenpixels.h"

#include "hook.h"
#include "logging.h"
#include <GL/gl.h>
#include "../external/SDL1.h" // SDL_Surface
#include "sdlwindows.h"
#include <vector>
#include <string.h> // memcpy

bool useGL;
bool inited = false;

/* Temporary pixel arrays */
std::vector<uint8_t> glpixels;
std::vector<uint8_t> winpixels;

/* Original function pointers */
namespace orig {
    static void (*SDL_GL_GetDrawableSize)(SDL_Window* window, int* w, int* h);
    static SDL1::SDL_Surface* (*SDL_GetVideoSurface)(void);
    static int (*SDL_LockSurface)(SDL1::SDL_Surface* surface);
    static void (*SDL_UnlockSurface)(SDL1::SDL_Surface* surface);
    static int (*SDL_RenderReadPixels)(SDL_Renderer*, const SDL_Rect*, Uint32, void*, int);
    static Uint32 (*SDL_GetWindowPixelFormat)(SDL_Window* window);
    static SDL_Renderer* (*SDL_GetRenderer)(SDL_Window* window);
    static int (*SDL_GetRendererOutputSize)(SDL_Renderer* renderer, int* w, int* h);
    static SDL_Texture* (*SDL_CreateTexture)(SDL_Renderer* renderer, Uint32 format, int access, int w, int h);
    static int (*SDL_UpdateTexture)(SDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
    static int (*SDL_RenderCopy)(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
    static void (*SDL_DestroyTexture)(SDL_Texture* texture);

    static void (*glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);
    static void (*glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data);
    static void (*glWindowPos2i)(GLint x, GLint y);
}

/* Video dimensions */
int width, height;
int size;

SDL_Renderer* renderer;
int pixelSize = 0;

int initScreenPixels(SDL_Window* window, bool opengl, int *pwidth, int *pheight)
{
    if (inited) {
        if (pwidth) *pwidth = width;
        if (pheight) *pheight = height;
        return 0;
    }

    /* If the game uses openGL, the method to capture the screen will be different */
    useGL = opengl;

    /* Link the required functions, and get the window dimensions */
    if (SDLver == 1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        if (!useGL) {
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

        width = surf->w;
        height = surf->h;
    }
    else if (SDLver == 2) {

        if (useGL) {
            LINK_NAMESPACE_SDL2(SDL_GL_GetDrawableSize);

            /* Get information about the current screen */
            if (!orig::SDL_GL_GetDrawableSize) {
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Need function SDL_GL_GetDrawableSize.");
                return -1;
            }

            orig::SDL_GL_GetDrawableSize(window, &width, &height);
            pixelSize = 4;
        }
        else {
            LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
            LINK_NAMESPACE_SDL2(SDL_GetRenderer);
            LINK_NAMESPACE_SDL2(SDL_GetRendererOutputSize);
            LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
            LINK_NAMESPACE_SDL2(SDL_CreateTexture);
            LINK_NAMESPACE_SDL2(SDL_UpdateTexture);
            LINK_NAMESPACE_SDL2(SDL_RenderCopy);
            LINK_NAMESPACE_SDL2(SDL_DestroyTexture);

            Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(window);
            pixelSize = sdlpixfmt & 0xFF;

            /* Get surface from window */
            if (!orig::SDL_GetRendererOutputSize) {
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Need function SDL_GetWindowSize.");
                return -1;
            }

            renderer = orig::SDL_GetRenderer(window);
            orig::SDL_GetRendererOutputSize(renderer, &width, &height);
        }
    }
    else {
        debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "Unknown SDL version.");
        return -1;
    }

    /* Output dimensions if asked */
    if (pwidth) *pwidth = width;
    if (pheight) *pheight = height;

    /* Allocate an array of pixels */
    size = width * height * pixelSize;
    winpixels.resize(size);

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglog(LCF_DUMP | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return -1;
    }

    if (useGL) {
        LINK_NAMESPACE(glReadPixels, "libGL");
        LINK_NAMESPACE(glDrawPixels, "libGL");
        LINK_NAMESPACE(glWindowPos2i, "libGL");

        /* Do we already have access to the glReadPixels function? */
        if (!orig::glReadPixels || !orig::glDrawPixels || !orig::glWindowPos2i) {
            debuglog(LCF_DUMP | LCF_OGL | LCF_ERROR, "Could not load function gl*.");
            return -1;
        }

        /* Allocate another pixels array,
         * because the image will need to be flipped.
         */
        glpixels.resize(size);
    }

    inited = true;
    return 0;
}

void finiScreenPixels()
{
    inited = false;
}

#ifdef LIBTAS_ENABLE_AVDUMPING

AVPixelFormat getPixelFormat(SDL_Window* window)
{
    MYASSERT(inited)

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

int getScreenPixels(const uint8_t* orig_plane[], int orig_stride[])
{
    MYASSERT(inited)

    int pitch = pixelSize * width;

    if (useGL) {
        /* TODO: Check that the openGL dimensions did not change in between */

        /* We access to the image pixels directly using glReadPixels */
        orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, glpixels.data());
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

    if (orig_plane)
        orig_plane[0] = const_cast<const uint8_t*>(winpixels.data());
    if (orig_stride)
        orig_stride[0] = pitch;

    return 0;
}

int setScreenPixels(SDL_Window* window) {
    MYASSERT(inited)

    int pitch = pixelSize * width;

    if (useGL) {
        orig::glWindowPos2i(0, 0);
        orig::glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, const_cast<const GLvoid*>(static_cast<GLvoid*>(glpixels.data())));
        if (SDLver == 1) {
            NATIVECALL(SDL_GL_SwapBuffers());
        }
        if (SDLver == 2) {
            NATIVECALL(SDL_GL_SwapWindow(window));
        }
    }

    else {
        if (SDLver == 1) {
            /* Not tested !! */
            debuglog(LCF_DUMP | LCF_UNTESTED | LCF_FRAME, "Set SDL1_Surface pixels");

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
            memcpy(surf1->pixels, winpixels.data(), size);

            /* Unlock surface */
            orig::SDL_UnlockSurface(surf1);

            NATIVECALL(SDL_Flip(surf1));
        }

        if (SDLver == 2) {
            /* Checking for a size modification */
            int cw, ch;
            orig::SDL_GetRendererOutputSize(renderer, &cw, &ch);
            if ((cw != width) || (ch != height)) {
                debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",cw,",",ch,")");
                return -1;
            }

            /* We must set our pixels into an SDL texture before drawing on screen */
            Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(window);
            SDL_Texture* texture = orig::SDL_CreateTexture(renderer, sdlpixfmt,
                SDL_TEXTUREACCESS_STREAMING, width, height);
            orig::SDL_UpdateTexture(texture, NULL, winpixels.data(), pitch);
            orig::SDL_RenderCopy(renderer, texture, NULL, NULL);
            orig::SDL_DestroyTexture(texture);
            NATIVECALL(SDL_RenderPresent(renderer));
        }
    }

    return 0;
}
