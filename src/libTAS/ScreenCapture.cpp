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

#include "ScreenCapture.h"

#include "hook.h"
#include "sdlversion.h"
#include "logging.h"
#include "../external/SDL1.h" // SDL_Surface
#include "opengl_helpers.h"
#include "global.h"

#include <cstring> // memcpy

namespace libtas {

/* Original function pointers */
namespace orig {
    static SDL1::SDL_Surface* (*SDL_GetVideoSurface)(void);
    static int (*SDL_LockSurface)(SDL1::SDL_Surface* surface);
    static void (*SDL_UnlockSurface)(SDL1::SDL_Surface* surface);
}

DEFINE_ORIG_POINTER(SDL_GetWindowSize);
DEFINE_ORIG_POINTER(SDL_RenderReadPixels);
DEFINE_ORIG_POINTER(SDL_GetWindowPixelFormat);
DEFINE_ORIG_POINTER(SDL_GetRenderer);
DEFINE_ORIG_POINTER(SDL_GetRendererOutputSize);
DEFINE_ORIG_POINTER(SDL_CreateTexture);
DEFINE_ORIG_POINTER(SDL_UpdateTexture);
DEFINE_ORIG_POINTER(SDL_RenderCopy);
DEFINE_ORIG_POINTER(SDL_DestroyTexture);

DEFINE_ORIG_POINTER(glGetIntegerv);
DEFINE_ORIG_POINTER(glReadPixels);
DEFINE_ORIG_POINTER(glDrawPixels);
DEFINE_ORIG_POINTER(glRasterPos2i);
DEFINE_ORIG_POINTER(glClear);
DEFINE_ORIG_POINTER(glClearColor);
DEFINE_ORIG_POINTER(glFinish);
DEFINE_ORIG_POINTER(glGetError);
DEFINE_ORIG_POINTER(glBindTexture);
DEFINE_ORIG_POINTER(glTexParameteri);
DEFINE_ORIG_POINTER(glTexImage2D);
DEFINE_ORIG_POINTER(glBegin);
DEFINE_ORIG_POINTER(glTexCoord2f);
DEFINE_ORIG_POINTER(glVertex2f);
DEFINE_ORIG_POINTER(glEnd);
DEFINE_ORIG_POINTER(glGenTextures);
DEFINE_ORIG_POINTER(glDeleteTextures);

bool ScreenCapture::inited = false;

/* Temporary pixel arrays */
std::vector<uint8_t> ScreenCapture::glpixels;
std::vector<uint8_t> ScreenCapture::winpixels;

/* Video dimensions */
int ScreenCapture::width, ScreenCapture::height, ScreenCapture::pitch;
unsigned int ScreenCapture::size;
int ScreenCapture::pixelSize;

GLuint ScreenCapture::screenTex = 0;
SDL_Window* ScreenCapture::sdl_window;
SDL_Renderer* ScreenCapture::sdl_renderer;

int ScreenCapture::init(SDL_Window* window)
{
    if (inited) {
        return 0;
    }

    sdl_window = window;

    /* Link the required functions, and get the window dimensions */
    if (game_info.video & GameInfo::OPENGL) {
        LINK_NAMESPACE(glGetIntegerv, "libGL");
        LINK_NAMESPACE(glReadPixels, "libGL");
        LINK_NAMESPACE(glDrawPixels, "libGL");
        LINK_NAMESPACE(glRasterPos2i, "libGL");
        LINK_NAMESPACE(glClear, "libGL");
        LINK_NAMESPACE(glClearColor, "libGL");
        LINK_NAMESPACE(glFinish, "libGL");
        LINK_NAMESPACE(glGetError, "libGL");
        LINK_NAMESPACE(glBindTexture, "libGL");
        LINK_NAMESPACE(glTexParameteri, "libGL");
        LINK_NAMESPACE(glTexImage2D, "libGL");
        LINK_NAMESPACE(glBegin, "libGL");
        LINK_NAMESPACE(glTexCoord2f, "libGL");
        LINK_NAMESPACE(glVertex2f, "libGL");
        LINK_NAMESPACE(glEnd, "libGL");
        LINK_NAMESPACE(glGenTextures, "libGL");
        LINK_NAMESPACE(glDeleteTextures, "libGL");

        /* OpenGL viewport size and window size can differ. Here we capture the
         * screen, so we want the window size.
         * TODO: Don't use viewport but only SDL/Xlib functions
         */
        if (game_info.video & GameInfo::SDL2) {
            LINK_NAMESPACE_SDL2(SDL_GetWindowSize);

            orig::SDL_GetWindowSize(sdl_window, &width, &height);
        }
        else {
            GLint viewport[4];
            orig::glGetIntegerv(GL_VIEWPORT, viewport);
            width = viewport[2];
            height = viewport[3];
        }

        /* TODO: Is this always 4 for OpenGL? */
        pixelSize = 4;

        /* Do we already have access to the glReadPixels function? */
        if (!orig::glGetIntegerv || !orig::glReadPixels ||
            !orig::glDrawPixels || !orig::glClear) {
            debuglog(LCF_WINDOW | LCF_OGL | LCF_ERROR, "Could not load function gl*.");
            return -1;
        }

        /* Create the screen texture */
        if (screenTex == 0) {
            orig::glGenTextures(1, &screenTex);
        }
    }

    else if (game_info.video & GameInfo::SDL1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        LINK_NAMESPACE_SDL1(SDL_LockSurface);
        LINK_NAMESPACE_SDL1(SDL_UnlockSurface);

        /* Get dimensions from the window surface */
        if (!orig::SDL_GetVideoSurface) {
            debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "Need function SDL_GetVideoSurface.");
            return -1;
        }

        SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();

        pixelSize = surf->format->BytesPerPixel;
        width = surf->w;
        height = surf->h;
    }

    else if (game_info.video & GameInfo::SDL2) {
        LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
        LINK_NAMESPACE_SDL2(SDL_GetRenderer);
        LINK_NAMESPACE_SDL2(SDL_GetRendererOutputSize);
        LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
        LINK_NAMESPACE_SDL2(SDL_CreateTexture);
        LINK_NAMESPACE_SDL2(SDL_UpdateTexture);
        LINK_NAMESPACE_SDL2(SDL_RenderCopy);
        LINK_NAMESPACE_SDL2(SDL_DestroyTexture);

        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl_window);
        pixelSize = sdlpixfmt & 0xFF;

        /* Get surface from window */
        if (!orig::SDL_GetRendererOutputSize) {
            debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "Need function SDL_GetRendererOutputSize.");
            return -1;
        }

        sdl_renderer = orig::SDL_GetRenderer(sdl_window);
        orig::SDL_GetRendererOutputSize(sdl_renderer, &width, &height);
    }

    /* We don't allocate the array of pixels here, we are doing a lazy
     * allocation when we will need it.
     */
    size = width * height * pixelSize;
    pitch = pixelSize * width;

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglog(LCF_WINDOW | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return -1;
    }

    debuglog(LCF_WINDOW, "Inited Screen Capture with dimensions (", width, ",", height, ")");

    inited = true;
    return 0;
}

void ScreenCapture::fini()
{
    winpixels.clear();
    glpixels.clear();

    /* Delete the OpenGL screen texture */
    if (screenTex != 0) {
        orig::glDeleteTextures(1, &screenTex);
        screenTex = 0;
    }

    inited = false;
}

void ScreenCapture::reinit(SDL_Window* window)
{
    if (inited) {
        fini();
        init(window);
    }
}

void ScreenCapture::getDimensions(int& w, int& h) {
    w = width;
    h = height;
}

#ifdef LIBTAS_ENABLE_AVDUMPING

AVPixelFormat ScreenCapture::getPixelFormat()
{
    MYASSERT(inited)

    if (game_info.video & GameInfo::OPENGL) {
        return AV_PIX_FMT_RGBA;
    }

    if (game_info.video & GameInfo::SDL1) {
        // SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
        debuglog(LCF_DUMP | LCF_SDL | LCF_TODO, "We assumed pixel format is RGBA");
        return AV_PIX_FMT_RGBA; // TODO
    }

    if (game_info.video & GameInfo::SDL2) {
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl_window);
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

int ScreenCapture::getPixels(const uint8_t* orig_plane[], int orig_stride[])
{
    MYASSERT(inited)

    /* Lazy allocations */
    if (winpixels.size() != size)
        winpixels.resize(size);

    if (game_info.video & GameInfo::OPENGL) {
        /* Allocate another pixels array,
         * because the image will need to be flipped.
         */
        if (glpixels.size() != size)
            glpixels.resize(size);

        /* TODO: Check that the openGL dimensions did not change in between */

        /* We access to the image pixels directly using glReadPixels */
        orig::glGetError();
        orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, glpixels.data());
        orig::glFinish();
        GLenum glerror = orig::glGetError();
        while (glerror != GL_NO_ERROR) {
            debuglog(LCF_ERROR | LCF_OGL, "OpenGL error: ", glerror);
            glerror = orig::glGetError();
        }

        if (orig_plane) {
            /*
             * Flip image horizontally
             * This is because OpenGL has a different reference point
             * Code taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
             */

            for (int line = 0; line < height; line++) {
                int pos = line * pitch;
                memcpy(&winpixels[pos], &glpixels[(size-pos)-pitch], pitch);
            }
        }
    }

    else {
        /* Not tested !! */
        debuglog(LCF_DUMP | LCF_UNTESTED | LCF_FRAME, "Access SDL_Surface pixels for video dump");

        if (game_info.video & GameInfo::SDL1) {
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

        if (game_info.video & GameInfo::SDL2) {
            /* Checking for a size modification */
            int cw, ch;
            orig::SDL_GetRendererOutputSize(sdl_renderer, &cw, &ch);
            if ((cw != width) || (ch != height)) {
                debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",cw,",",ch,")");
                return -1;
            }

            orig::SDL_RenderReadPixels(sdl_renderer, NULL, 0, winpixels.data(), pitch);
        }
    }

    if (orig_plane)
        orig_plane[0] = const_cast<const uint8_t*>(winpixels.data());
    if (orig_stride)
        orig_stride[0] = pitch;

    return 0;
}

int ScreenCapture::setPixels(bool update) {
    MYASSERT(inited)

    if (game_info.video & GameInfo::OPENGL) {
        orig::glGetError();

        // orig::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        //orig::glClear(GL_COLOR_BUFFER_BIT);
//        orig::glFinish();
        orig::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        orig::glRasterPos2i(0, 0);


        /* Load the screen pixels into a texture */

        enterGLRender();
        orig::glBindTexture(GL_TEXTURE_2D, screenTex);

        orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //if (update) {
            orig::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, const_cast<const GLvoid*>(static_cast<GLvoid*>(glpixels.data())));
        //}

        orig::glBegin(GL_QUADS);
        {
            orig::glTexCoord2f(0,1); orig::glVertex2f(0, 0);
            orig::glTexCoord2f(1,1); orig::glVertex2f(width, 0);
            orig::glTexCoord2f(1,0); orig::glVertex2f(width, height);
            orig::glTexCoord2f(0,0); orig::glVertex2f(0, height);
        }
        orig::glEnd();

        exitGLRender();

        //orig::glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, const_cast<const GLvoid*>(static_cast<GLvoid*>(glpixels.data())));
        GLenum glerror = orig::glGetError();
        while (glerror != GL_NO_ERROR) {
            debuglog(LCF_ERROR | LCF_OGL, "OpenGL error: ", glerror);
            glerror = orig::glGetError();
        }
    }

    else {
        if (game_info.video & GameInfo::SDL1) {
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
        }

        if (game_info.video & GameInfo::SDL2) {
            /* Checking for a size modification */
            int cw, ch;
            orig::SDL_GetRendererOutputSize(sdl_renderer, &cw, &ch);
            if ((cw != width) || (ch != height)) {
                debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",cw,",",ch,")");
                return -1;
            }

            /* We must set our pixels into an SDL texture before drawing on screen */
            Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl_window);
            SDL_Texture* texture = orig::SDL_CreateTexture(sdl_renderer, sdlpixfmt,
                SDL_TEXTUREACCESS_STREAMING, width, height);
            orig::SDL_UpdateTexture(texture, NULL, winpixels.data(), pitch);
            orig::SDL_RenderCopy(sdl_renderer, texture, NULL, NULL);
            orig::SDL_DestroyTexture(texture);
        }
    }

    return 0;
}

}
