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

#include "ScreenCapture_GL.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"

#include <cstring> // memcpy
#define GL_GLEXT_PROTOTYPES
#ifdef __unix__
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#define GL_CALL(FUNC, ARGS) \
do { \
    LINK_NAMESPACE(FUNC, "GL"); \
    orig::FUNC ARGS; \
    GLenum error; \
    if ((error = orig::glGetError()) != GL_NO_ERROR) \
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, #FUNC " failed with error %d", error); \
} while (0) \

namespace libtas {

DECLARE_ORIG_POINTER(glReadPixels)
DECLARE_ORIG_POINTER(glGenFramebuffers)
DECLARE_ORIG_POINTER(glBindBuffer)
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
DECLARE_ORIG_POINTER(glPixelStorei)
DECLARE_ORIG_POINTER(glFramebufferTexture2D)
DECLARE_ORIG_POINTER(glTexImage2D)
DECLARE_ORIG_POINTER(glTexParameteri)
DECLARE_ORIG_POINTER(glBindTexture)
DECLARE_ORIG_POINTER(glGenTextures)
DECLARE_ORIG_POINTER(glDeleteTextures)
DECLARE_ORIG_POINTER(glClear)

int ScreenCapture_GL::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    pixelSize = 4;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_GL::initScreenSurface()
{
    /* Reset error flag */
    LINK_NAMESPACE(glGetError, "GL");
    orig::glGetError();

    /* Generate FBO and RBO */
    GLint draw_buffer, read_buffer;
    GL_CALL(glGetIntegerv, (GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer));
    GL_CALL(glGetIntegerv, (GL_READ_FRAMEBUFFER_BINDING, &read_buffer));

    if (screenFBO == 0) {
        GL_CALL(glGenFramebuffers, (1, &screenFBO));
    }

    GL_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, screenFBO));
    
    GL_CALL(glGenTextures, (1, &screenTex));
    GL_CALL(glBindTexture, (GL_TEXTURE_2D, screenTex));
    GL_CALL(glTexImage2D, (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glFramebufferTexture2D, (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex, 0));

    if (screenRBO == 0) {
        GL_CALL(glGenRenderbuffers, (1, &screenRBO));
    }

    GL_CALL(glBindRenderbuffer, (GL_RENDERBUFFER, screenRBO));
    GL_CALL(glRenderbufferStorage, (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
    GL_CALL(glFramebufferRenderbuffer, (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screenRBO));

    GL_CALL(glBindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));        
    GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));

    gllinepixels.resize(pitch);
}

void ScreenCapture_GL::destroyScreenSurface()
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
    if (screenTex != 0) {
        LINK_NAMESPACE(glDeleteTextures, "GL");
        orig::glDeleteTextures(1, &screenTex);
        screenTex = 0;
    }
}

uint32_t ScreenCapture_GL::screenTexture()
{
    return screenTex;
}

const char* ScreenCapture_GL::getPixelFormat()
{
    return "RGBA";
}

int ScreenCapture_GL::copyScreenToSurface()
{
    GlobalNative gn;

    LINK_NAMESPACE(glEnable, "GL");
    LINK_NAMESPACE(glDisable, "GL");
    LINK_NAMESPACE(glIsEnabled, "GL");
    LINK_NAMESPACE(glGetIntegerv, "GL");
    LINK_NAMESPACE(glClear, "GL");

    /* Disable sRGB if needed */
    GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
    if (isFramebufferSrgb)
        orig::glDisable(GL_FRAMEBUFFER_SRGB);
    
    /* Copy the original draw/read framebuffers */
    GLint draw_buffer, read_buffer;
    orig::glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
    orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);
    
    orig::glGetError();
    
    GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, 0));
    GL_CALL(glBindFramebuffer, (GL_DRAW_FRAMEBUFFER, screenFBO));
    GL_CALL(glBlitFramebuffer, (0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    
    /* Restore the original draw/read framebuffers */
    GL_CALL(glBindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));
    GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));
    
    if (isFramebufferSrgb)
        orig::glEnable(GL_FRAMEBUFFER_SRGB);

    return size;
}

int ScreenCapture_GL::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    LINK_NAMESPACE(glBindBuffer, "GL");
    LINK_NAMESPACE(glEnable, "GL");
    LINK_NAMESPACE(glDisable, "GL");
    LINK_NAMESPACE(glIsEnabled, "GL");
    LINK_NAMESPACE(glGetIntegerv, "GL");
    LINK_NAMESPACE(glPixelStorei, "GL");

    /* Disable sRGB if needed */
    GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
    if (isFramebufferSrgb)
        orig::glDisable(GL_FRAMEBUFFER_SRGB);

    /* Copy the original read framebuffer */
    GLint read_buffer;
    orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);

    /* Copy the original pixel buffer */
    GLint pixel_buffer;
    orig::glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pixel_buffer);

    /* Copy the original pack row length */
    GLint pack_row;
    orig::glGetIntegerv(GL_PACK_ROW_LENGTH, &pack_row);

    orig::glGetError();

    GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, screenFBO));

    if (pixel_buffer != 0)
        orig::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    if (pack_row != 0)
        orig::glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    GL_CALL(glReadPixels, (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, winpixels.data()));

    if (pack_row != 0)
        orig::glPixelStorei(GL_PACK_ROW_LENGTH, pack_row);

    if (pixel_buffer != 0)
        orig::glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);

    GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));

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

    return size;
}

int ScreenCapture_GL::copySurfaceToScreen()
{
    GlobalNative gn;

    // LINK_NAMESPACE(glEnable, "GL");
    // LINK_NAMESPACE(glDisable, "GL");
    // LINK_NAMESPACE(glIsEnabled, "GL");
    // LINK_NAMESPACE(glGetIntegerv, "GL");
    // 
    // GLboolean isFramebufferSrgb = orig::glIsEnabled(GL_FRAMEBUFFER_SRGB);
    // if (isFramebufferSrgb)
    //     orig::glDisable(GL_FRAMEBUFFER_SRGB);
    // 
    // /* Copy the original draw/read framebuffers */
    // GLint draw_buffer, read_buffer;
    // orig::glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
    // orig::glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);
    // 
    // orig::glGetError();
    // GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, screenFBO));
    // GL_CALL(glBindFramebuffer, (GL_DRAW_FRAMEBUFFER, 0));
    // GL_CALL(glBlitFramebuffer, (0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    // 
    // /* Restore the original draw/read framebuffers */
    // GL_CALL(glBindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));
    // GL_CALL(glBindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));
    // 
    // if (isFramebufferSrgb)
    //     orig::glEnable(GL_FRAMEBUFFER_SRGB);

    return 0;
}

void ScreenCapture_GL::clearScreen()
{
    LINK_NAMESPACE(glClear, "GL");
    orig::glClear(GL_COLOR_BUFFER_BIT);
}

}
