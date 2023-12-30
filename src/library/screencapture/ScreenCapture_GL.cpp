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

int ScreenCapture_GL::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;

    pixelSize = 4;

    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_GL::initScreenSurface()
{
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
}

const char* ScreenCapture_GL::getPixelFormat()
{
    return "RGBA";
}

int ScreenCapture_GL::copyScreenToSurface()
{
    GlobalNative gn;

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

    LINK_NAMESPACE(glReadPixels, "GL");
    LINK_NAMESPACE(glBindBuffer, "GL");
    LINK_NAMESPACE(glBindFramebuffer, "GL");
    LINK_NAMESPACE(glBlitFramebuffer, "GL");
    LINK_NAMESPACE(glEnable, "GL");
    LINK_NAMESPACE(glDisable, "GL");
    LINK_NAMESPACE(glIsEnabled, "GL");
    LINK_NAMESPACE(glGetIntegerv, "GL");
    LINK_NAMESPACE(glPixelStorei, "GL");

    GLenum error;

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

    orig::glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFBO);
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glBindFramebuffer failed with error %d", error);

    if (pixel_buffer != 0)
        orig::glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    if (pack_row != 0)
        orig::glPixelStorei(GL_PACK_ROW_LENGTH, 0);

    orig::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, winpixels.data());
    if ((error = orig::glGetError()) != GL_NO_ERROR)
        debuglogstdio(LCF_WINDOW | LCF_OGL | LCF_ERROR, "glReadPixels failed with error %d", error);

    if (pack_row != 0)
        orig::glPixelStorei(GL_PACK_ROW_LENGTH, pack_row);

    if (pixel_buffer != 0)
        orig::glBindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);

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

    return size;
}

int ScreenCapture_GL::copySurfaceToScreen()
{
    GlobalNative gn;

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

    return 0;
}

}
