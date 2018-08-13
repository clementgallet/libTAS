/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "global.h"

#include <cstring> // memcpy
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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
DEFINE_ORIG_POINTER(SDL_GetError);

DEFINE_ORIG_POINTER(glGetIntegerv);
DEFINE_ORIG_POINTER(glReadPixels);
DEFINE_ORIG_POINTER(glGenFramebuffers);
DEFINE_ORIG_POINTER(glBindFramebuffer);
DEFINE_ORIG_POINTER(glDeleteFramebuffers);
DEFINE_ORIG_POINTER(glGenRenderbuffers);
DEFINE_ORIG_POINTER(glBindRenderbuffer);
DEFINE_ORIG_POINTER(glDeleteRenderbuffers);
DEFINE_ORIG_POINTER(glRenderbufferStorage);
DEFINE_ORIG_POINTER(glFramebufferRenderbuffer);
DEFINE_ORIG_POINTER(glBlitFramebuffer);

static bool inited = false;

/* Temporary pixel arrays */
static std::vector<uint8_t> glpixels;
static std::vector<uint8_t> winpixels;

/* Video dimensions */
static int width, height, pitch;
static unsigned int size;
static int pixelSize;

/* OpenGL framebuffer */
static GLuint screenFBO = 0;

/* OpenGL render buffer */
static GLuint screenRBO = 0;

/* SDL2 screen texture */
static SDL_Texture* screenSDLTex = nullptr;

/* SDL window if any */
static SDL_Window* sdl_window;

/* SDL2 renderer if any */
static SDL_Renderer* sdl_renderer;

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
        LINK_NAMESPACE(glGenFramebuffers, "libGL");
        LINK_NAMESPACE(glBindFramebuffer, "libGL");
        LINK_NAMESPACE(glDeleteFramebuffers, "libGL");
        LINK_NAMESPACE(glGenRenderbuffers, "libGL");
        LINK_NAMESPACE(glBindRenderbuffer, "libGL");
        LINK_NAMESPACE(glDeleteRenderbuffers, "libGL");
        LINK_NAMESPACE(glRenderbufferStorage, "libGL");
        LINK_NAMESPACE(glFramebufferRenderbuffer, "libGL");
        LINK_NAMESPACE(glBlitFramebuffer, "libGL");

        /* OpenGL viewport size and window size can differ. Here we capture the
         * screen, so we want the window size.
         * TODO: Don't use viewport but only SDL/Xlib functions
         */
        if (game_info.video & GameInfo::SDL2) {
            if (!window) {
                /* Init must provide a sdl_window pointer */
                return -1;
            }
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

        /* Generate FBO and RBO */
        if (screenFBO == 0) {
            orig::glGenFramebuffers(1, &screenFBO);
        }
        orig::glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

        if (screenRBO == 0) {
            orig::glGenRenderbuffers(1, &screenRBO);
        }
        orig::glBindRenderbuffer(GL_RENDERBUFFER, screenRBO);

        orig::glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
        orig::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, screenRBO);
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
        LINK_NAMESPACE_SDL2(SDL_GetError);

        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl_window);
        pixelSize = sdlpixfmt & 0xFF;

        sdl_renderer = orig::SDL_GetRenderer(sdl_window);
        int ret = orig::SDL_GetRendererOutputSize(sdl_renderer, &width, &height);
        if (ret < 0) {
            debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_GetRendererOutputSize failed: ", orig::SDL_GetError());
        }

        /* Create the screen texture */
        if (!screenSDLTex) {
            screenSDLTex = orig::SDL_CreateTexture(sdl_renderer, sdlpixfmt,
                SDL_TEXTUREACCESS_STREAMING, width, height);
            if (!screenSDLTex) {
                debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_CreateTexture failed: ", orig::SDL_GetError());
            }
        }
    }

    size = width * height * pixelSize;
    pitch = pixelSize * width;

    winpixels.resize(size);
    if (game_info.video & GameInfo::OPENGL) {
        glpixels.resize(size);
    }

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

    /* Delete openGL framebuffers */
    if (screenFBO != 0) {
        orig::glDeleteFramebuffers(1, &screenFBO);
        screenFBO = 0;
    }
    if (screenRBO != 0) {
        orig::glDeleteRenderbuffers(1, &screenRBO);
        screenRBO = 0;
    }

    /* Delete the SDL2 screen texture */
    if (screenSDLTex) {
        orig::SDL_DestroyTexture(screenSDLTex);
        screenSDLTex = nullptr;
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

const char* ScreenCapture::getPixelFormat()
{
    MYASSERT(inited)

    if (game_info.video & GameInfo::OPENGL) {
        return "BGRA";
    }

    if (game_info.video & GameInfo::SDL1) {
        // SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
        debuglog(LCF_DUMP | LCF_SDL | LCF_TODO, "We assumed pixel format is RGBA");
        return "BGRA"; // TODO
    }

    if (game_info.video & GameInfo::SDL2) {
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(sdl_window);
        /* TODO: There are probably helper functions to build pixel
         * format constants from channel masks
         */
        switch (sdlpixfmt) {
            case SDL_PIXELFORMAT_RGBA8888:
                debuglog(LCF_DUMP | LCF_SDL, "  RGBA");
                return "BGRA";
            case SDL_PIXELFORMAT_BGRA8888:
                debuglog(LCF_DUMP | LCF_SDL, "  BGRA");
                return "RGBA";
            case SDL_PIXELFORMAT_ARGB8888:
                debuglog(LCF_DUMP | LCF_SDL, "  ARGB");
                return "ABGR";
            case SDL_PIXELFORMAT_ABGR8888:
                debuglog(LCF_DUMP | LCF_SDL, "  ABGR");
                return "ARGB";
            case SDL_PIXELFORMAT_RGB24:
                debuglog(LCF_DUMP | LCF_SDL, "  RGB24");
                return "24BG";
            case SDL_PIXELFORMAT_BGR24:
                debuglog(LCF_DUMP | LCF_SDL, "  BGR24");
                return "RAW ";
            default:
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "  Unsupported pixel format");
        }
    }

    return "BGRA";
}

int ScreenCapture::getPixels(uint8_t **pixels)
{
    if (!inited)
        return 0;

    if (game_info.video & GameInfo::OPENGL) {
        /* Copy the default framebuffer to our FBO */
        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
        orig::glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        orig::glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (pixels) {

            /* We need to recover the pixels for encoding */
            orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
            orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, glpixels.data());
            orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

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

            int ret = orig::SDL_RenderReadPixels(sdl_renderer, NULL, 0, winpixels.data(), pitch);
            if (ret < 0) {
                debuglog(LCF_DUMP | LCF_SDL | LCF_ERROR, "SDL_RenderReadPixels failed: ", orig::SDL_GetError());
            }
        }
    }

    if (pixels) {
        *pixels = winpixels.data();
    }

    return size;
}

int ScreenCapture::setPixels(bool same) {
    if (!inited)
        return 0;

    static bool last_same = false;

    if (game_info.video & GameInfo::OPENGL) {
        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        orig::glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        orig::glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

            int ret;

            /* Check if we need to update our texture */
            if (!same || !last_same) {
                /* We must set our pixels into an SDL texture before drawing on screen */
                ret = orig::SDL_UpdateTexture(screenSDLTex, NULL, winpixels.data(), pitch);
                if (ret < 0) {
                    debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_UpdateTexture failed: ", orig::SDL_GetError());
                }
                last_same = same;
            }

            ret = orig::SDL_RenderCopy(sdl_renderer, screenSDLTex, NULL, NULL);
            if (ret < 0) {
                debuglog(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_RenderCopy failed: ", orig::SDL_GetError());
            }
        }
    }

    return 0;
}

}
