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

#include "rendering/openglloader.h"

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
    LINK_GL_POINTER(FUNC); \
    glProcs.FUNC ARGS; \
    GLenum error; \
    if ((error = glProcs.GetError()) != GL_NO_ERROR) \
        LOG(LL_ERROR, LCF_WINDOW | LCF_OGL, #FUNC " failed with error %d", error); \
} while (0) \

namespace libtas {

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
    glProcs.GetError();

    /* Generate FBO and RBO */
    GLint draw_buffer, read_buffer;
    GL_CALL(GetIntegerv, (GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer));
    GL_CALL(GetIntegerv, (GL_READ_FRAMEBUFFER_BINDING, &read_buffer));

    if (screenFBO == 0) {
        GL_CALL(GenFramebuffers, (1, &screenFBO));
    }

    GL_CALL(BindFramebuffer, (GL_FRAMEBUFFER, screenFBO));
    
    GL_CALL(GenTextures, (1, &screenTex));
    GL_CALL(BindTexture, (GL_TEXTURE_2D, screenTex));
    GL_CALL(TexImage2D, (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    GL_CALL(TexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(TexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(FramebufferTexture2D, (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex, 0));

    if (screenRBO == 0) {
        GL_CALL(GenRenderbuffers, (1, &screenRBO));
    }

    GL_CALL(BindRenderbuffer, (GL_RENDERBUFFER, screenRBO));
    GL_CALL(RenderbufferStorage, (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
    GL_CALL(FramebufferRenderbuffer, (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screenRBO));

    GL_CALL(BindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));        
    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));

    gllinepixels.resize(pitch);
}

void ScreenCapture_GL::destroyScreenSurface()
{
    /* Delete openGL framebuffers */
    if (screenFBO != 0) {
        glProcs.DeleteFramebuffers(1, &screenFBO);
        screenFBO = 0;
    }
    if (screenRBO != 0) {
        glProcs.DeleteRenderbuffers(1, &screenRBO);
        screenRBO = 0;
    }
    if (screenTex != 0) {
        glProcs.DeleteTextures(1, &screenTex);
        screenTex = 0;
    }
}

uint64_t ScreenCapture_GL::screenTexture()
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

    /* Disable sRGB if needed */
    GLboolean isFramebufferSrgb = glProcs.IsEnabled(GL_FRAMEBUFFER_SRGB);
    if (isFramebufferSrgb)
        glProcs.Disable(GL_FRAMEBUFFER_SRGB);
    
    /* Copy the original draw/read framebuffers */
    GLint draw_buffer, read_buffer;
    glProcs.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
    glProcs.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);
    
    glProcs.GetError();
    
    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, 0));
    GL_CALL(BindFramebuffer, (GL_DRAW_FRAMEBUFFER, screenFBO));
    GL_CALL(BlitFramebuffer, (0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    
    /* Restore the original draw/read framebuffers */
    GL_CALL(BindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));
    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));
    
    if (isFramebufferSrgb)
        glProcs.Enable(GL_FRAMEBUFFER_SRGB);

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

    /* Disable sRGB if needed */
    GLboolean isFramebufferSrgb = glProcs.IsEnabled(GL_FRAMEBUFFER_SRGB);
    if (isFramebufferSrgb)
        glProcs.Disable(GL_FRAMEBUFFER_SRGB);

    /* Copy the original read framebuffer */
    GLint read_buffer;
    glProcs.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);

    /* Copy the original pixel buffer */
    GLint pixel_buffer;
    glProcs.GetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pixel_buffer);

    /* Copy the original pack row length */
    GLint pack_row;
    glProcs.GetIntegerv(GL_PACK_ROW_LENGTH, &pack_row);

    glProcs.GetError();

    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, screenFBO));

    if (pixel_buffer != 0)
        glProcs.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    if (pack_row != 0)
        glProcs.PixelStorei(GL_PACK_ROW_LENGTH, 0);

    GL_CALL(ReadPixels, (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, winpixels.data()));

    if (pack_row != 0)
        glProcs.PixelStorei(GL_PACK_ROW_LENGTH, pack_row);

    if (pixel_buffer != 0)
        glProcs.BindBuffer(GL_PIXEL_PACK_BUFFER, pixel_buffer);

    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));

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
        glProcs.Enable(GL_FRAMEBUFFER_SRGB);

    return size;
}

int ScreenCapture_GL::copySurfaceToScreen()
{
    GlobalNative gn;

    GLboolean isFramebufferSrgb = glProcs.IsEnabled(GL_FRAMEBUFFER_SRGB);
    if (isFramebufferSrgb)
        glProcs.Disable(GL_FRAMEBUFFER_SRGB);
    
    /* Copy the original draw/read framebuffers */
    GLint draw_buffer, read_buffer;
    glProcs.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_buffer);
    glProcs.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_buffer);
    
    glProcs.GetError();
    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, screenFBO));
    GL_CALL(BindFramebuffer, (GL_DRAW_FRAMEBUFFER, 0));
    GL_CALL(BlitFramebuffer, (0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
    
    /* Restore the original draw/read framebuffers */
    GL_CALL(BindFramebuffer, (GL_DRAW_FRAMEBUFFER, draw_buffer));
    GL_CALL(BindFramebuffer, (GL_READ_FRAMEBUFFER, read_buffer));
    
    if (isFramebufferSrgb)
        glProcs.Enable(GL_FRAMEBUFFER_SRGB);

    return 0;
}

void ScreenCapture_GL::clearScreen()
{
    glProcs.Clear(GL_COLOR_BUFFER_BIT);
}

}
