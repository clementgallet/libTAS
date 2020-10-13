/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "logging.h"
#include "../external/SDL1.h" // SDL_Surface
#include "global.h"
#include "encoding/AVEncoder.h"

#include <cstring> // memcpy
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include "vdpauwrappers.h"

namespace libtas {

/* Original function pointers */
namespace orig {
    static void (*SDL1_FreeSurface)(SDL1::SDL_Surface *surface);
    static SDL1::SDL_Surface* (*SDL_GetVideoSurface)(void);
    static int (*SDL1_LockSurface)(SDL1::SDL_Surface* surface);
    static void (*SDL1_UnlockSurface)(SDL1::SDL_Surface* surface);
    static int (*SDL1_UpperBlit)(SDL1::SDL_Surface *src, SDL1::SDL_Rect *srcrect, SDL1::SDL_Surface *dst, SDL1::SDL_Rect *dstrect);
    static uint8_t (*SDL1_SetClipRect)(SDL1::SDL_Surface *surface, const SDL1::SDL_Rect *rect);
    static void (*SDL1_GetClipRect)(SDL1::SDL_Surface *surface, SDL1::SDL_Rect *rect);
    static int (*SDL_SetAlpha)(SDL1::SDL_Surface *surface, Uint32 flag, Uint8 alpha);
    static SDL1::SDL_Surface *(*SDL_DisplayFormat)(SDL1::SDL_Surface *surface);
}

DECLARE_ORIG_POINTER(SDL_RenderReadPixels);
DECLARE_ORIG_POINTER(SDL_GetWindowPixelFormat);
DECLARE_ORIG_POINTER(SDL_GetRenderer);
DECLARE_ORIG_POINTER(SDL_CreateTexture);
DECLARE_ORIG_POINTER(SDL_LockTexture);
DECLARE_ORIG_POINTER(SDL_UnlockTexture);
DECLARE_ORIG_POINTER(SDL_RenderCopy);
DECLARE_ORIG_POINTER(SDL_DestroyTexture);
DECLARE_ORIG_POINTER(SDL_GetError);
DECLARE_ORIG_POINTER(SDL_GetWindowSurface);
DECLARE_ORIG_POINTER(SDL_ConvertSurfaceFormat);
DECLARE_ORIG_POINTER(SDL_FreeSurface);
DECLARE_ORIG_POINTER(SDL_SetClipRect);
DECLARE_ORIG_POINTER(SDL_GetClipRect);
DECLARE_ORIG_POINTER(SDL_LockSurface);
DECLARE_ORIG_POINTER(SDL_UnlockSurface);
DECLARE_ORIG_POINTER(SDL_UpperBlit)

DECLARE_ORIG_POINTER(glReadPixels);
DECLARE_ORIG_POINTER(glGenFramebuffers);
DECLARE_ORIG_POINTER(glBindFramebuffer);
DECLARE_ORIG_POINTER(glDeleteFramebuffers);
DECLARE_ORIG_POINTER(glGenRenderbuffers);
DECLARE_ORIG_POINTER(glBindRenderbuffer);
DECLARE_ORIG_POINTER(glDeleteRenderbuffers);
DECLARE_ORIG_POINTER(glRenderbufferStorage);
DECLARE_ORIG_POINTER(glFramebufferRenderbuffer);
DECLARE_ORIG_POINTER(glBlitFramebuffer);
DECLARE_ORIG_POINTER(glEnable);
DECLARE_ORIG_POINTER(glDisable);
DECLARE_ORIG_POINTER(glIsEnabled);
DECLARE_ORIG_POINTER(glGetIntegerv);
DECLARE_ORIG_POINTER(glGetError);
DECLARE_ORIG_POINTER(VdpOutputSurfaceGetParameters);
DECLARE_ORIG_POINTER(VdpOutputSurfaceCreate);
DECLARE_ORIG_POINTER(VdpOutputSurfaceDestroy);
DECLARE_ORIG_POINTER(VdpOutputSurfaceRenderOutputSurface);
DECLARE_ORIG_POINTER(VdpOutputSurfaceGetBitsNative);

DEFINE_ORIG_POINTER(XGetGeometry);

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

/* SDL1 screen surface */
static SDL1::SDL_Surface* screenSDL1Surf = nullptr;

/* SDL2 screen surface */
static SDL_Surface* screenSDL2Surf = nullptr;

/* SDL2 screen texture */
static SDL_Texture* screenSDLTex = nullptr;

/* SDL2 renderer if any */
static SDL_Renderer* sdl_renderer;

/* VDPAU screen surface */
static VdpOutputSurface screenVDPAUSurf;

int ScreenCapture::init()
{
    if (inited) {
        return 0;
    }

    /* Don't initialize if window is not registered */
    if (gameXWindows.empty())
        return 0;

    /* Get the window dimensions */
    LINK_NAMESPACE_GLOBAL(XGetGeometry);
    unsigned int w = 0, h = 0, border_width, depth;
    int x, y;
    Window root;
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (gameDisplays[i]) {
            orig::XGetGeometry(gameDisplays[i], gameXWindows.front(), &root, &x, &y, &w, &h, &border_width, &depth);
            break;
        }
    }
    width = w;
    height = h;

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return -1;
    }

    /* Get window color depth */
    if (game_info.video & GameInfo::VDPAU) {
        VdpRGBAFormat rgba_format;
        unsigned int uw, uh;
        orig::VdpOutputSurfaceGetParameters(vdpSurface, &rgba_format, &uw, &uh);
        if (rgba_format == VDP_RGBA_FORMAT_A8) {
            pixelSize = 1;
        }
        else {
            pixelSize = 4;
        }
        /* Also overwrite the dimensions */
        width = uw;
        height = uh;
    }
    else if ((game_info.video & GameInfo::SDL2_RENDERER) || (game_info.video & GameInfo::SDL2_SURFACE)) {
        LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(gameSDLWindow);
        pixelSize = sdlpixfmt & 0xFF;
    }
    else if (game_info.video & GameInfo::OPENGL) {
        pixelSize = 4;
    }
    else if (game_info.video & GameInfo::SDL1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
        if (!surf) {
            return -1;
        }
        pixelSize = surf->format->BytesPerPixel;
    }
    else if (game_info.video & GameInfo::XSHM) {
        pixelSize = gameXImage->bits_per_pixel / 8;
        /* Also overwrite the dimensions */
        width = gameXImage->width;
        height = gameXImage->height;
    }
    else {
        pixelSize = depth / 8; /* depth is in bits/pixels */
    }

    size = width * height * pixelSize;
    pitch = pixelSize * width;

    winpixels.resize(size);

    initScreenSurface();

    debuglogstdio(LCF_WINDOW, "Inited Screen Capture with dimensions (%d,%d)", width, height);

    inited = true;
    return 0;
}

void ScreenCapture::initScreenSurface()
{
    /* Set up a backup surface/framebuffer */
    if (game_info.video & GameInfo::VDPAU) {
        VdpStatus status = orig::VdpOutputSurfaceCreate(vdpDevice, VDP_RGBA_FORMAT_B8G8R8A8, width, height, &screenVDPAUSurf);
        if (status != VDP_STATUS_OK) {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceCreate failed with status %d", status);
            return;
        }
    }
    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        LINK_NAMESPACE_SDL2(SDL_GetRenderer);
        LINK_NAMESPACE_SDL2(SDL_CreateTexture);
        LINK_NAMESPACE_SDL2(SDL_GetError);
        LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);

        /* Create the screen texture */
        sdl_renderer = orig::SDL_GetRenderer(gameSDLWindow);
        if (!sdl_renderer) {
            debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_GetRenderer failed: %d", orig::SDL_GetError());
        }
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(gameSDLWindow);
        if (!screenSDLTex) {
            screenSDLTex = orig::SDL_CreateTexture(sdl_renderer, sdlpixfmt,
                SDL_TEXTUREACCESS_STREAMING, width, height);
            if (!screenSDLTex) {
                debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_CreateTexture failed: %d", orig::SDL_GetError());
            }
        }
    }
    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
        LINK_NAMESPACE_SDL2(SDL_ConvertSurfaceFormat);
        LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);

        SDL_Surface *surf = orig::SDL_GetWindowSurface(gameSDLWindow);
        screenSDL2Surf = orig::SDL_ConvertSurfaceFormat(surf, orig::SDL_GetWindowPixelFormat(gameSDLWindow), 0);
    }
    else if (game_info.video & GameInfo::OPENGL) {
        /* Generate FBO and RBO */
        LINK_NAMESPACE(glGenFramebuffers, "GL");
        LINK_NAMESPACE(glBindFramebuffer, "GL");
        LINK_NAMESPACE(glGenRenderbuffers, "GL");
        LINK_NAMESPACE(glBindRenderbuffer, "GL");
        LINK_NAMESPACE(glRenderbufferStorage, "GL");
        LINK_NAMESPACE(glFramebufferRenderbuffer, "GL");
        LINK_NAMESPACE(glGetIntegerv, "GL");
        LINK_NAMESPACE(glGetError, "GL");

        /* Reset error flag */
        GLenum error;
        orig::glGetError();

        GLint draw_buffer, read_buffer;
        orig::glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glGetIntegerv failed with error %d", error);

        orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glGetIntegerv failed with error %d", error);

        if (screenFBO == 0) {
            orig::glGenFramebuffers(1, &screenFBO);
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glGenFramebuffers failed with error %d", error);
        }
        orig::glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        if (screenRBO == 0) {
            orig::glGenRenderbuffers(1, &screenRBO);
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glGenRenderbuffers failed with error %d", error);
        }
        orig::glBindRenderbuffer(GL_RENDERBUFFER, screenRBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindRenderbuffer failed with error %d", error);

        orig::glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glRenderbufferStorage failed with error %d", error);

        orig::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, screenRBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glFramebufferRenderbuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        glpixels.resize(size);
    }

    else if (game_info.video & GameInfo::SDL1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        LINK_NAMESPACE_SDL1(SDL_SetAlpha);
        LINK_NAMESPACE_SDL1(SDL_DisplayFormat);

        SDL1::SDL_Surface *surf = orig::SDL_GetVideoSurface();
        screenSDL1Surf = orig::SDL_DisplayFormat(surf);

        /* Disable alpha blending for that texture */
        if (screenSDL1Surf->flags & SDL1::SDL1_SRCALPHA) {
            orig::SDL_SetAlpha(screenSDL1Surf, 0, 0);
        }
    }
}


void ScreenCapture::fini()
{
    winpixels.clear();
    glpixels.clear();

    destroyScreenSurface();

    inited = false;
}

void ScreenCapture::destroyScreenSurface()
{
    /* Delete openGL framebuffers */
    if (screenFBO != 0) {
        LINK_NAMESPACE(glDeleteFramebuffers, "GL");
        orig::glDeleteFramebuffers(1, &screenFBO);
        screenFBO = 0;
    }
    if (screenRBO != 0) {
        LINK_NAMESPACE(glDeleteRenderbuffers, "GL");
        orig::glDeleteRenderbuffers(1, &screenRBO);
        screenRBO = 0;
    }

    /* Delete the SDL1 screen surface */
    if (screenSDL1Surf) {
        link_function((void**)&orig::SDL1_FreeSurface, "SDL_FreeSurface", "libSDL-1.2.so.0");
        orig::SDL1_FreeSurface(screenSDL1Surf);
        screenSDL1Surf = nullptr;
    }

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

    /* Delete the SDL2 screen surface */
    if (screenVDPAUSurf) {
        orig::VdpOutputSurfaceDestroy(screenVDPAUSurf);
        screenSDL2Surf = 0;
    }
}

void ScreenCapture::resize(int w, int h)
{
    if (!inited) {
        return;
    }

    /* Don't resize if window is not registered */
    if (gameXWindows.empty()) {
        return;
    }

    if (width == w && height == h) {
        return;
    }

    destroyScreenSurface();

    width = w;
    height = h;
    size = width * height * pixelSize;
    pitch = pixelSize * width;

    winpixels.resize(size);

    initScreenSurface();

    /* We need to close the dumping if needed, and open a new one */
    if (shared_config.av_dumping) {
        avencoder.reset(new AVEncoder());
    }

    debuglogstdio(LCF_WINDOW, "Resize Screen Capture with new dimensions (%d,%d) and size %d", width, height, size);
}

bool ScreenCapture::isInited()
{
    return inited;
}

void ScreenCapture::getDimensions(int& w, int& h) {
    w = width;
    h = height;
}

const char* ScreenCapture::getPixelFormat()
{
    MYASSERT(inited)

    if (game_info.video & GameInfo::VDPAU) {
        VdpRGBAFormat rgba_format;
        unsigned int uw, uh;
        orig::VdpOutputSurfaceGetParameters(vdpSurface, &rgba_format, &uw, &uh);
        switch (rgba_format) {
            case VDP_RGBA_FORMAT_B8G8R8A8:
                return "BGRA";
            case VDP_RGBA_FORMAT_R8G8B8A8:
                return "RGBA";
            default:
                debuglogstdio(LCF_DUMP | LCF_ERROR, "  Unsupported pixel format %d", rgba_format);
        }
    }

    else if ((game_info.video & GameInfo::SDL2_RENDERER) || (game_info.video & GameInfo::SDL2_SURFACE)) {
        LINK_NAMESPACE_SDL2(SDL_GetWindowPixelFormat);
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(gameSDLWindow);
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
    }

    else if (game_info.video & GameInfo::OPENGL) {
        return "RGBA";
    }

    else if (game_info.video & GameInfo::SDL1) {
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

    else if (game_info.video & GameInfo::XSHM) {
        /* Apparently, it will only be RGB or BGR depending on the endianness
         * of the machine. */

        if (gameXImage->bits_per_pixel == 24) {
            if (gameXImage->byte_order == LSBFirst)
                return "24BG";
            else
                return "RAW ";
        }
        else if (gameXImage->bits_per_pixel == 32) {
            if (gameXImage->byte_order == LSBFirst)
                return "BGR\0";
            else
                return "RGB\0";
        }
        else {
            debuglogstdio(LCF_DUMP | LCF_ERROR, "  Unsupported pixel format");
        }
    }

    return "RGBA";
}

int ScreenCapture::storePixels()
{
    return getPixels(nullptr, true);
}

int ScreenCapture::getPixels(uint8_t **pixels, bool draw)
{
    if (!inited)
        return 0;

    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    if (game_info.video & GameInfo::VDPAU) {
        /* Copy to our screen surface */
        VdpStatus status = orig::VdpOutputSurfaceRenderOutputSurface(screenVDPAUSurf, nullptr, vdpSurface, nullptr, nullptr, nullptr, 0);
        if (status != VDP_STATUS_OK) {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderOutputSurface failed with status %d", status);
        }

        /* Copy pixels */
        void* const pix = reinterpret_cast<void* const>(winpixels.data());
        unsigned int pp = pitch;
        status = orig::VdpOutputSurfaceGetBitsNative(screenVDPAUSurf, nullptr, &pix, &pp);
    }

    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
        LINK_NAMESPACE_SDL2(SDL_LockTexture);
        LINK_NAMESPACE_SDL2(SDL_UnlockTexture);

        int ret = orig::SDL_RenderReadPixels(sdl_renderer, NULL, 0, winpixels.data(), pitch);
        if (ret < 0) {
            debuglogstdio(LCF_DUMP | LCF_SDL | LCF_ERROR, "SDL_RenderReadPixels failed: %d", orig::SDL_GetError());
        }
        void* tex_pixels;
        int tex_pitch;
        orig::SDL_LockTexture(screenSDLTex, nullptr, &tex_pixels, &tex_pitch);
        memcpy(tex_pixels, winpixels.data(), size);
        orig::SDL_UnlockTexture(screenSDLTex);
    }

    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        debuglogstdio(LCF_DUMP, "Access SDL_Surface pixels for video dump");

        LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
        LINK_NAMESPACE_SDL2(SDL_LockSurface);
        LINK_NAMESPACE_SDL2(SDL_UnlockSurface);
        LINK_NAMESPACE_SDL2(SDL_UpperBlit);

        /* Get surface from window */
        SDL_Surface* surf2 = orig::SDL_GetWindowSurface(gameSDLWindow);

        /* Checking for a size modification */
        int cw = surf2->w;
        int ch = surf2->h;
        if ((cw != width) || (ch != height)) {
            debuglogstdio(LCF_DUMP | LCF_ERROR, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, cw, ch);
            return -1;
        }

        orig::SDL_UpperBlit(surf2, nullptr, screenSDL2Surf, nullptr);

        if (pixels) {
            /* We must lock the surface before accessing the raw pixels */
            if (SDL_MUSTLOCK(screenSDL2Surf))
                orig::SDL_LockSurface(screenSDL2Surf);

            /* I know memcpy is not recommended for vectors... */
            memcpy(winpixels.data(), screenSDL2Surf->pixels, size);

            /* Unlock surface */
            if (SDL_MUSTLOCK(screenSDL2Surf))
                orig::SDL_UnlockSurface(screenSDL2Surf);
        }
    }

    else if (game_info.video & GameInfo::OPENGL) {
        LINK_NAMESPACE(glReadPixels, "GL");
        LINK_NAMESPACE(glBindFramebuffer, "GL");
        LINK_NAMESPACE(glBlitFramebuffer, "GL");
        LINK_NAMESPACE(glEnable, "GL");
        LINK_NAMESPACE(glDisable, "GL");
        LINK_NAMESPACE(glIsEnabled, "GL");
        LINK_NAMESPACE(glGetIntegerv, "GL");

        GLenum error;

        /* Copy the default framebuffer to our FBO */
        GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
        if (isFramebufferSrgb)
            orig::glDisable(GL_FRAMEBUFFER_SRGB);

        /* Copy the original draw/read framebuffers */
        GLint draw_buffer, read_buffer;
        orig::glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
        orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);

        orig::glGetError();
        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBlitFramebuffer failed with error %d", error);

        /* Restore the original draw/read framebuffers */
        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        if (pixels) {

            /* We need to recover the pixels for encoding */
            orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

            orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, glpixels.data());
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glReadPixels failed with error %d", error);

            orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
            if ((error = orig::glGetError()) != GL_NO_ERROR)
                debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glReadPixels failed with error %d", error);

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

        if (isFramebufferSrgb)
            orig::glEnable(GL_FRAMEBUFFER_SRGB);
    }

    else if (game_info.video & GameInfo::SDL1) {
        /* Not tested !! */
        debuglogstdio(LCF_DUMP, "Access SDL_Surface pixels for video dump");

        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        link_function((void**)&orig::SDL1_LockSurface, "SDL_LockSurface", "libSDL-1.2.so.0");
        link_function((void**)&orig::SDL1_UnlockSurface, "SDL_UnlockSurface", "libSDL-1.2.so.0");
        link_function((void**)&orig::SDL1_UpperBlit, "SDL_UpperBlit", "libSDL-1.2.so.0");
        LINK_NAMESPACE_SDL1(SDL_SetAlpha);

        /* Get surface from window */
        SDL1::SDL_Surface* surf1 = orig::SDL_GetVideoSurface();

        /* Checking for a size modification */
        int cw = surf1->w;
        int ch = surf1->h;
        if ((cw != width) || (ch != height)) {
            debuglogstdio(LCF_DUMP | LCF_ERROR, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, cw, ch);
            return -1;
        }

        if (surf1->flags & SDL1::SDL1_SRCALPHA) {
            orig::SDL_SetAlpha(surf1, 0, 0);
            orig::SDL1_UpperBlit(surf1, nullptr, screenSDL1Surf, nullptr);
            orig::SDL_SetAlpha(surf1, SDL1::SDL1_SRCALPHA, 0);
        }
        else {
            orig::SDL1_UpperBlit(surf1, nullptr, screenSDL1Surf, nullptr);
        }

        if (pixels) {
            /* We must lock the surface before accessing the raw pixels */
            int ret = orig::SDL1_LockSurface(screenSDL1Surf);
            if (ret != 0) {
                debuglogstdio(LCF_DUMP | LCF_ERROR, "Could not lock SDL surface");
                return -1;
            }

            /* I know memcpy is not recommended for vectors... */
            memcpy(winpixels.data(), screenSDL1Surf->pixels, size);

            /* Unlock surface */
            orig::SDL1_UnlockSurface(screenSDL1Surf);
        }
    }

    else if (game_info.video & GameInfo::XSHM) {
        if ((gameXImage->width != width) || (gameXImage->height != height)) {
            debuglogstdio(LCF_DUMP | LCF_ERROR, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, gameXImage->width, gameXImage->height);
            return -1;
        }
        memcpy(winpixels.data(), gameXImage->data, size);
    }

    return size;
}

int ScreenCapture::setPixels() {
    if (!inited)
        return 0;

    GlobalNative gn;

    if (game_info.video & GameInfo::VDPAU) {
        VdpStatus status = orig::VdpOutputSurfaceRenderOutputSurface(vdpSurface, nullptr, screenVDPAUSurf, nullptr, nullptr, nullptr, 0);
        if (status != VDP_STATUS_OK) {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderOutputSurface failed with status %d", status);
        }
    }

    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        LINK_NAMESPACE_SDL2(SDL_RenderCopy);

        int ret;

        ret = orig::SDL_RenderCopy(sdl_renderer, screenSDLTex, NULL, NULL);
        if (ret < 0) {
            debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_RenderCopy failed: %d", orig::SDL_GetError());
        }
    }

    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
        LINK_NAMESPACE_SDL2(SDL_UpperBlit);
        LINK_NAMESPACE_SDL2(SDL_GetClipRect);
        LINK_NAMESPACE_SDL2(SDL_SetClipRect);

        debuglogstdio(LCF_DUMP, "Set SDL1_Surface pixels");

        /* Get surface from window */
        SDL_Surface* surf2 = orig::SDL_GetWindowSurface(gameSDLWindow);

        /* Save and restore the clip rectangle */
        SDL_Rect clip_rect;
        orig::SDL_GetClipRect(surf2, &clip_rect);
        orig::SDL_SetClipRect(surf2, nullptr);
        orig::SDL_UpperBlit(screenSDL2Surf, nullptr, surf2, nullptr);
        orig::SDL_SetClipRect(surf2, &clip_rect);
    }

    else if (game_info.video & GameInfo::OPENGL) {
        LINK_NAMESPACE(glBindFramebuffer, "GL");
        LINK_NAMESPACE(glBlitFramebuffer, "GL");
        LINK_NAMESPACE(glEnable, "GL");
        LINK_NAMESPACE(glDisable, "GL");
        LINK_NAMESPACE(glIsEnabled, "GL");
        LINK_NAMESPACE(glGetIntegerv, "GL");

        GLenum error;

        GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
        if (isFramebufferSrgb)
            orig::glDisable(GL_FRAMEBUFFER_SRGB);

        /* Copy the original draw/read framebuffers */
        GLint draw_buffer, read_buffer;
        orig::glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
        orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);

        orig::glGetError();
        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBlitFramebuffer failed with error %d", error);

        /* Restore the original draw/read framebuffers */
        orig::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        if (isFramebufferSrgb)
            orig::glEnable(GL_FRAMEBUFFER_SRGB);
    }

    else if (game_info.video & GameInfo::SDL1) {
        LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
        // link_function((void**)&orig::SDL1_LockSurface, "SDL_LockSurface", "libSDL-1.2.so.0")
        // link_function((void**)&orig::SDL1_UnlockSurface, "SDL_UnlockSurface", "libSDL-1.2.so.0")
        link_function((void**)&orig::SDL1_GetClipRect, "SDL_GetClipRect", "libSDL-1.2.so.0");
        link_function((void**)&orig::SDL1_SetClipRect, "SDL_SetClipRect", "libSDL-1.2.so.0");
        link_function((void**)&orig::SDL1_UpperBlit, "SDL_UpperBlit", "libSDL-1.2.so.0");

        /* Not tested !! */
        debuglogstdio(LCF_DUMP, "Set SDL1_Surface pixels");

        /* Get surface from window */
        SDL1::SDL_Surface* surf1 = orig::SDL_GetVideoSurface();

        /* Save and restore the clip rectangle */
        SDL1::SDL_Rect clip_rect;
        orig::SDL1_GetClipRect(surf1, &clip_rect);
        orig::SDL1_SetClipRect(surf1, nullptr);
        orig::SDL1_UpperBlit(screenSDL1Surf, nullptr, surf1, nullptr);
        orig::SDL1_SetClipRect(surf1, &clip_rect);

    }

    else if (game_info.video & GameInfo::XSHM) {
        memcpy(gameXImage->data, winpixels.data(), size);
    }

    return 0;
}

}
