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

#include <SDL2/SDL.h>
#include <vector>
#include <cstring> // memcpy
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include "vdpauwrappers.h"
#include "vulkanwrappers.h"

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

DECLARE_ORIG_POINTER(SDL_RenderReadPixels)
DECLARE_ORIG_POINTER(SDL_GetWindowPixelFormat)
DECLARE_ORIG_POINTER(SDL_GetRenderer)
DECLARE_ORIG_POINTER(SDL_CreateTexture)
DECLARE_ORIG_POINTER(SDL_LockTexture)
DECLARE_ORIG_POINTER(SDL_UnlockTexture)
DECLARE_ORIG_POINTER(SDL_RenderCopy)
DECLARE_ORIG_POINTER(SDL_DestroyTexture)
DECLARE_ORIG_POINTER(SDL_GetError)
DECLARE_ORIG_POINTER(SDL_GetWindowSurface)
DECLARE_ORIG_POINTER(SDL_ConvertSurfaceFormat)
DECLARE_ORIG_POINTER(SDL_FreeSurface)
DECLARE_ORIG_POINTER(SDL_SetClipRect)
DECLARE_ORIG_POINTER(SDL_GetClipRect)
DECLARE_ORIG_POINTER(SDL_LockSurface)
DECLARE_ORIG_POINTER(SDL_UnlockSurface)
DECLARE_ORIG_POINTER(SDL_UpperBlit)

DECLARE_ORIG_POINTER(glReadPixels)
DECLARE_ORIG_POINTER(glGenFramebuffers)
DECLARE_ORIG_POINTER(glBindFramebuffer)
DECLARE_ORIG_POINTER(glDeleteFramebuffers)
DECLARE_ORIG_POINTER(glGenRenderbuffers)
DECLARE_ORIG_POINTER(glBindRenderbuffer)
DECLARE_ORIG_POINTER(glDeleteRenderbuffers)
DECLARE_ORIG_POINTER(glRenderbufferStorage)
DECLARE_ORIG_POINTER(glFramebufferRenderbuffer)
DECLARE_ORIG_POINTER(glBlitFramebuffer)
DECLARE_ORIG_POINTER(glEnable)
DECLARE_ORIG_POINTER(glDisable)
DECLARE_ORIG_POINTER(glIsEnabled)
DECLARE_ORIG_POINTER(glGetIntegerv)
DECLARE_ORIG_POINTER(glGetError)
DECLARE_ORIG_POINTER(VdpOutputSurfaceGetParameters)
DECLARE_ORIG_POINTER(VdpOutputSurfaceCreate)
DECLARE_ORIG_POINTER(VdpOutputSurfaceDestroy)
DECLARE_ORIG_POINTER(VdpOutputSurfaceRenderOutputSurface)
DECLARE_ORIG_POINTER(VdpOutputSurfaceGetBitsNative)

DECLARE_ORIG_POINTER(vkCreateImage)
DECLARE_ORIG_POINTER(vkGetImageMemoryRequirements)
DECLARE_ORIG_POINTER(vkAllocateMemory)
DECLARE_ORIG_POINTER(vkBindImageMemory)
DECLARE_ORIG_POINTER(vkUnmapMemory)
DECLARE_ORIG_POINTER(vkFreeMemory)
DECLARE_ORIG_POINTER(vkDestroyImage)
DECLARE_ORIG_POINTER(vkAllocateCommandBuffers)
DECLARE_ORIG_POINTER(vkBeginCommandBuffer)
DECLARE_ORIG_POINTER(vkCmdPipelineBarrier)
DECLARE_ORIG_POINTER(vkCmdBlitImage)
DECLARE_ORIG_POINTER(vkCmdCopyImage)
DECLARE_ORIG_POINTER(vkEndCommandBuffer)
DECLARE_ORIG_POINTER(vkQueueWaitIdle)
DECLARE_ORIG_POINTER(vkQueueSubmit)
DECLARE_ORIG_POINTER(vkFreeCommandBuffers)
DECLARE_ORIG_POINTER(vkGetImageSubresourceLayout)
DECLARE_ORIG_POINTER(vkMapMemory)
DECLARE_ORIG_POINTER(vkAcquireNextImageKHR)

DEFINE_ORIG_POINTER(XGetGeometry)

static bool inited = false;

/* Stored pixel array for use with the video encoder */
static std::vector<uint8_t> winpixels;

/* Single line of pixels to swap GL array that has different reference point */
static std::vector<uint8_t> gllinepixels;

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

/* Vulkan screen image */
static VkImage vkScreenImage;
static VkDeviceMemory vkScreenImageMemory;

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
    else if (game_info.video & GameInfo::VULKAN) {
        pixelSize = 4;
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
            debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_GetRenderer failed: %s", orig::SDL_GetError());
        }
        Uint32 sdlpixfmt = orig::SDL_GetWindowPixelFormat(gameSDLWindow);
        if (!screenSDLTex) {
            screenSDLTex = orig::SDL_CreateTexture(sdl_renderer, sdlpixfmt,
                SDL_TEXTUREACCESS_STREAMING, width, height);
            if (!screenSDLTex) {
                debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_CreateTexture failed: %s", orig::SDL_GetError());
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

        gllinepixels.resize(pitch);
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
    else if (game_info.video & GameInfo::VULKAN) {
        LINK_NAMESPACE(vkCreateImage, "vulkan");
        LINK_NAMESPACE(vkGetImageMemoryRequirements, "vulkan");
        LINK_NAMESPACE(vkAllocateMemory, "vulkan");
        LINK_NAMESPACE(vkBindImageMemory, "vulkan");
        
        VkResult res;
        
        /* Create the image info */
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = 1;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        
        /* Create the image */
        if ((res = orig::vkCreateImage(vk::device, &imageInfo, nullptr, &vkScreenImage)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkCreateImage failed with error %d", res);
        }
        
        /* Create memory to back up the image */
        VkMemoryRequirements memRequirements;
        orig::vkGetImageMemoryRequirements(vk::device, vkScreenImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        // Memory must be host visible to copy from
        allocInfo.memoryTypeIndex = vk::getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if ((res = orig::vkAllocateMemory(vk::device, &allocInfo, nullptr, &vkScreenImageMemory)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkAllocateMemory failed with error %d", res);
        }

        orig::vkBindImageMemory(vk::device, vkScreenImage, vkScreenImageMemory, 0);
    }
}


void ScreenCapture::fini()
{
    winpixels.clear();
    gllinepixels.clear();

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
    
    /* Delete the Vulkan image */
    if (vkScreenImageMemory) {
        LINK_NAMESPACE(vkFreeMemory, "vulkan");
        orig::vkFreeMemory(vk::device, vkScreenImageMemory, nullptr);
        vkScreenImageMemory = VK_NULL_HANDLE;
    }
    if (vkScreenImage) {
        LINK_NAMESPACE(vkDestroyImage, "vulkan");
        orig::vkDestroyImage(vk::device, vkScreenImage, nullptr);
        vkScreenImage = VK_NULL_HANDLE;
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

int ScreenCapture::getSize()
{
    return size;
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

    else if (game_info.video & GameInfo::VULKAN) {
        switch(vk::colorFormat) {
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SNORM:
                return "BGRA";
            default:
                debuglogstdio(LCF_DUMP | LCF_VULKAN | LCF_ERROR, "  Unsupported pixel format %d", vk::colorFormat);
                return "RGBA";
        }
    }

    return "RGBA";
}

int ScreenCapture::copyScreenToSurface()
{
    if (!inited)
        return 0;

    GlobalNative gn;

    if (game_info.video & GameInfo::VDPAU) {
        /* Copy to our screen surface */
        VdpStatus status = orig::VdpOutputSurfaceRenderOutputSurface(screenVDPAUSurf, nullptr, vdpSurface, nullptr, nullptr, nullptr, 0);
        if (status != VDP_STATUS_OK) {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderOutputSurface failed with status %d", status);
        }
    }

    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        LINK_NAMESPACE_SDL2(SDL_RenderReadPixels);
        LINK_NAMESPACE_SDL2(SDL_LockTexture);
        LINK_NAMESPACE_SDL2(SDL_UnlockTexture);

        /* I couldn't find a way to directly copy the screen to a texture.
         * Because apparently there is no way to access to the screen texture.
         * So copying the screen pixels into the texture. */
        void* tex_pixels;
        int tex_pitch;
        orig::SDL_LockTexture(screenSDLTex, nullptr, &tex_pixels, &tex_pitch);
        int ret = orig::SDL_RenderReadPixels(sdl_renderer, NULL, 0, tex_pixels, pitch);
        if (ret < 0) {
            debuglogstdio(LCF_DUMP | LCF_SDL | LCF_ERROR, "SDL_RenderReadPixels failed: %s", orig::SDL_GetError());
        }
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

        /* Disable sRGB if needed */
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
    }

    else if (game_info.video & GameInfo::XSHM) {
        if ((gameXImage->width != width) || (gameXImage->height != height)) {
            debuglogstdio(LCF_DUMP | LCF_ERROR, "Window coords have changed (%d,%d) -> (%d,%d)", width, height, gameXImage->width, gameXImage->height);
            return -1;
        }
        
        /* There is no designated surface for XShm, just copy to our array */
        memcpy(winpixels.data(), gameXImage->data, size);
    }

    else if (game_info.video & GameInfo::VULKAN) {
        LINK_NAMESPACE(vkAllocateCommandBuffers, "vulkan");
        LINK_NAMESPACE(vkBeginCommandBuffer, "vulkan");
        LINK_NAMESPACE(vkCmdPipelineBarrier, "vulkan");
        LINK_NAMESPACE(vkCmdCopyImage, "vulkan");
        LINK_NAMESPACE(vkEndCommandBuffer, "vulkan");
        LINK_NAMESPACE(vkQueueWaitIdle, "vulkan");
        LINK_NAMESPACE(vkQueueSubmit, "vulkan");
        LINK_NAMESPACE(vkFreeCommandBuffers, "vulkan");

        VkResult res;

        /* Create the command buffer */
        VkCommandBufferAllocateInfo allocInfo{};
        
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vk::commandPool;
        allocInfo.commandBufferCount = 1;
        
		VkCommandBuffer cmdBuffer;
        if ((res = orig::vkAllocateCommandBuffers(vk::device, &allocInfo, &cmdBuffer)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkAllocateCommandBuffers failed with error %d", res);
        }

		VkCommandBufferBeginInfo cmdBufInfo{};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if ((res = orig::vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkBeginCommandBuffer failed with error %d", res);
        }

        /* Transition destination image to transfer destination layout */
        VkImageMemoryBarrier dstBarrier{};
        dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        dstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.image = vkScreenImage;
        dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        dstBarrier.subresourceRange.baseMipLevel = 0;
        dstBarrier.subresourceRange.levelCount = 1;
        dstBarrier.subresourceRange.baseArrayLayer = 0;
        dstBarrier.subresourceRange.layerCount = 1;
        dstBarrier.srcAccessMask = 0;
        dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrier
        );

        VkImageMemoryBarrier srcBarrier{};
        srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        srcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.image = vk::swapchainImgs[vk::swapchainImgIndex];
        srcBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcBarrier.subresourceRange.baseMipLevel = 0;
        srcBarrier.subresourceRange.levelCount = 1;
        srcBarrier.subresourceRange.baseArrayLayer = 0;
        srcBarrier.subresourceRange.layerCount = 1;
        srcBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrier
        );

		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.extent.width = width;
		imageCopyRegion.extent.height = height;
		imageCopyRegion.extent.depth = 1;
    
		/* Issue the copy command */
		orig::vkCmdCopyImage(
			cmdBuffer,
			vk::swapchainImgs[vk::swapchainImgIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			vkScreenImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);

		/* Transition destination image to general layout, which is the required layout for mapping the image memory later on */
        dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrier
        );

        srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        srcBarrier.image = vk::swapchainImgs[vk::swapchainImgIndex];
        srcBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrier
        );

        /* Flush the command buffer */
        if ((res = orig::vkEndCommandBuffer(cmdBuffer)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        if ((res = orig::vkQueueSubmit(vk::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
        }

        orig::vkQueueWaitIdle(vk::graphicsQueue);
        orig::vkFreeCommandBuffers(vk::device, vk::commandPool, 1, &cmdBuffer);
    }
    
    return size;
}

int ScreenCapture::getPixelsFromSurface(uint8_t **pixels, bool draw)
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
        /* Copy pixels */
        void* const pix = reinterpret_cast<void* const>(winpixels.data());
        unsigned int pp = pitch;
        VdpStatus status = orig::VdpOutputSurfaceGetBitsNative(screenVDPAUSurf, nullptr, &pix, &pp);
        if (status != VDP_STATUS_OK) {
            debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceGetBitsNative failed with status %d", status);
        }
    }

    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        LINK_NAMESPACE_SDL2(SDL_LockTexture);
        LINK_NAMESPACE_SDL2(SDL_UnlockTexture);

        /* Access the texture and copy pixels */
        void* tex_pixels;
        int tex_pitch;
        orig::SDL_LockTexture(screenSDLTex, nullptr, &tex_pixels, &tex_pitch);
        memcpy(winpixels.data(), tex_pixels, size);
        orig::SDL_UnlockTexture(screenSDLTex);
    }

    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        /* We must lock the surface before accessing the raw pixels */
        if (SDL_MUSTLOCK(screenSDL2Surf))
            orig::SDL_LockSurface(screenSDL2Surf);

        /* I know memcpy is not recommended for vectors... */
        memcpy(winpixels.data(), screenSDL2Surf->pixels, size);

        /* Unlock surface */
        if (SDL_MUSTLOCK(screenSDL2Surf))
            orig::SDL_UnlockSurface(screenSDL2Surf);
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

        /* Disable sRGB if needed */
        GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
        if (isFramebufferSrgb)
            orig::glDisable(GL_FRAMEBUFFER_SRGB);

        /* Copy the original read framebuffer */
        GLint read_buffer;
        orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);

        orig::glGetError();

        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, winpixels.data());
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glReadPixels failed with error %d", error);

        orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, read_buffer);
        if ((error = orig::glGetError()) != GL_NO_ERROR)
            debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

        /*
         * Flip image horizontally in place
         * This is because OpenGL has a different reference point
         * Code taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
         */
        for (int line = 0; line < (height/2); line++) {
            int pos1 = line * pitch;
            int pos2 = (height-line-1) * pitch;
            memcpy(gllinepixels.data(), &winpixels[pos1], pitch);
            memcpy(&winpixels[pos1], &winpixels[pos2], pitch);
            memcpy(&winpixels[pos2], gllinepixels.data(), pitch);
        }

        if (isFramebufferSrgb)
            orig::glEnable(GL_FRAMEBUFFER_SRGB);
    }

    else if (game_info.video & GameInfo::SDL1) {
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

    else if (game_info.video & GameInfo::XSHM) {
        /* Nothing to do here, the surface is already stored in the pixel array */
    }

    else if (game_info.video & GameInfo::VULKAN) {
        LINK_NAMESPACE(vkGetImageSubresourceLayout, "vulkan");
        LINK_NAMESPACE(vkMapMemory, "vulkan");
        LINK_NAMESPACE(vkUnmapMemory, "vulkan");
        
        VkResult res;

        /* Get layout of the image (including row pitch) */
		VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		orig::vkGetImageSubresourceLayout(vk::device, vkScreenImage, &subResource, &subResourceLayout);

		/* Map image memory so we can start copying from it */
		const char* data;
        if ((res = orig::vkMapMemory(vk::device, vkScreenImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
        }
		data += subResourceLayout.offset;
        memcpy(winpixels.data(), data, size);
        
        orig::vkUnmapMemory(vk::device, vkScreenImageMemory);
    }
    
    return size;
}

int ScreenCapture::copySurfaceToScreen()
{
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
            debuglogstdio(LCF_WINDOW | LCF_SDL | LCF_ERROR, "SDL_RenderCopy to screen failed: %s", orig::SDL_GetError());
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

    else if (game_info.video & GameInfo::VULKAN) {
        LINK_NAMESPACE(vkAcquireNextImageKHR, "vulkan");
        LINK_NAMESPACE(vkAllocateCommandBuffers, "vulkan");
        LINK_NAMESPACE(vkBeginCommandBuffer, "vulkan");
        LINK_NAMESPACE(vkCmdPipelineBarrier, "vulkan");
        LINK_NAMESPACE(vkCmdBlitImage, "vulkan");
        LINK_NAMESPACE(vkCmdCopyImage, "vulkan");
        LINK_NAMESPACE(vkEndCommandBuffer, "vulkan");
        LINK_NAMESPACE(vkQueueWaitIdle, "vulkan");
        LINK_NAMESPACE(vkQueueSubmit, "vulkan");
        LINK_NAMESPACE(vkFreeCommandBuffers, "vulkan");

        /* Acquire an image from the swapchain */
        VkResult res = orig::vkAcquireNextImageKHR(vk::device, vk::swapchain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &vk::swapchainImgIndex);
        if ((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR))
            debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "vkAcquireNextImageKHR failed with error %d", res);

        debuglogstdio(LCF_WINDOW | LCF_VULKAN, "vkAcquireNextImageKHR called again. Returns image index %d", vk::swapchainImgIndex);
        
        /* Create the command buffer */
        VkCommandBufferAllocateInfo allocInfo{};
        
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vk::commandPool;
        allocInfo.commandBufferCount = 1;
        
        VkCommandBuffer cmdBuffer;
        if ((res = orig::vkAllocateCommandBuffers(vk::device, &allocInfo, &cmdBuffer)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkAllocateCommandBuffers failed with error %d", res);
        }

        VkCommandBufferBeginInfo cmdBufInfo{};
        cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if ((res = orig::vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkBeginCommandBuffer failed with error %d", res);
        }

        /* Transition destination image to transfer destination layout */
        VkImageMemoryBarrier dstBarrier{};
        dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        dstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBarrier.image = vk::swapchainImgs[vk::swapchainImgIndex];
        dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        dstBarrier.subresourceRange.baseMipLevel = 0;
        dstBarrier.subresourceRange.levelCount = 1;
        dstBarrier.subresourceRange.baseArrayLayer = 0;
        dstBarrier.subresourceRange.layerCount = 1;
        dstBarrier.srcAccessMask = 0;
        dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrier
        );

        VkImageMemoryBarrier srcBarrier{};
        srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        srcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBarrier.image = vkScreenImage;
        srcBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcBarrier.subresourceRange.baseMipLevel = 0;
        srcBarrier.subresourceRange.levelCount = 1;
        srcBarrier.subresourceRange.baseArrayLayer = 0;
        srcBarrier.subresourceRange.layerCount = 1;
        srcBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrier
        );

        /* Otherwise use image copy (requires us to manually flip components) */
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = width;
        imageCopyRegion.extent.height = height;
        imageCopyRegion.extent.depth = 1;
    
        /* Issue the copy command */
        orig::vkCmdCopyImage(
            cmdBuffer,
            vkScreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            vk::swapchainImgs[vk::swapchainImgIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);

        /* Transition destination image to general layout, which is the required layout for mapping the image memory later on */
        dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &dstBarrier
        );

        srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        srcBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        srcBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        orig::vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &srcBarrier
        );

        /* Flush the command buffer */
        if ((res = orig::vkEndCommandBuffer(cmdBuffer)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        if ((res = orig::vkQueueSubmit(vk::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS) {
            debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
        }

        orig::vkQueueWaitIdle(vk::graphicsQueue);
        orig::vkFreeCommandBuffers(vk::device, vk::commandPool, 1, &cmdBuffer);
    }

    return 0;
}

void ScreenCapture::restoreScreenState()
{
    if (!inited)
        return;

    if (game_info.video & GameInfo::VDPAU) {
        /* Probably not needed */
        // copySurfaceToScreen();        
    }
    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        /* Possibly needed */
        copySurfaceToScreen();
    }
    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        /* Definitively needed in some cases */
        copySurfaceToScreen();
    }

    else if (game_info.video & GameInfo::OPENGL) {
        /* Probably not needed */
        // copySurfaceToScreen();        
    }

    else if (game_info.video & GameInfo::SDL1) {
        /* Definitively needed in some cases */
        copySurfaceToScreen();
    }

    else if (game_info.video & GameInfo::XSHM) {
        /* Definitively needed in some cases */
        copySurfaceToScreen();
    }

    else if (game_info.video & GameInfo::VULKAN) {
        /* Must not do anything here, because of how Vulkan works */
    }
}

}
