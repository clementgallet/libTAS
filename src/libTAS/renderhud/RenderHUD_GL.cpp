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

#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD_GL.h"
#include "../opengl_helpers.h"
#include "../logging.h"
#include "../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(glGetIntegerv)
DEFINE_ORIG_POINTER(glGenTextures)
DEFINE_ORIG_POINTER(glBindTexture)
DEFINE_ORIG_POINTER(glTexParameteri)
DEFINE_ORIG_POINTER(glTexImage2D)
DEFINE_ORIG_POINTER(glBegin)
DEFINE_ORIG_POINTER(glTexCoord2f)
DEFINE_ORIG_POINTER(glVertex2f)
DEFINE_ORIG_POINTER(glEnd)

// namespace orig {
//     static void (*glGenTextures)(int n, unsigned int* tex);
//     static void (*glBindTexture)(int target, unsigned int tex);
//     static void (*glTexImage2D)(int, int, int, int, int, int, int, int, const void*);
//     static void (*glBegin)( int mode );
//     static void (*glEnd)( void );
//     static void (*glVertex2f)( float x, float y );
//     static void (*glTexCoord2f)( float s, float t );
//     static void (*glDeleteTextures)( int n, const unsigned int *textures);
//     static void (*glEnable)( int cap );
//     static void (*glDisable)( int cap );
//     static void (*glVertexPointer)(int size, int type, int stride, const void* pointer);
//     static void (*glDrawArrays)( int mode, int first, int count);
//     static void (*glMatrixMode)(int mode);
//     static void (*glPushMatrix)(void);
//     static void (*glPopMatrix)(void);
//     static void (*glLoadIdentity)(void);
//     static void (*glOrtho)(double left, double right, double bottom, double top, double near, double far);
//     static void (*glBlendFunc)(int sfactor, int dfactor);
//     static void (*glTexParameteri)(int target, int pname, int param);
//     static void (*glGetIntegerv)( int pname, GLint* data);
//     static void (*glGetBooleanv)( int pname, GLboolean* data);
//     static void (*glUseProgram)(unsigned int program);
// }

RenderHUD_GL::~RenderHUD_GL()
{
    /* We should destroy our created GL texture here,
     * but they are not handled yet.
     */
}

void RenderHUD_GL::init()
{
    RenderHUD::init();

    LINK_NAMESPACE(glGenTextures, "libGL");
    LINK_NAMESPACE(glBindTexture, "libGL");
    LINK_NAMESPACE(glTexImage2D, "libGL");
    LINK_NAMESPACE(glBegin, "libGL");
    LINK_NAMESPACE(glEnd, "libGL");
    LINK_NAMESPACE(glVertex2f, "libGL");
    LINK_NAMESPACE(glTexCoord2f, "libGL");
    // LINK_NAMESPACE(glDeleteTextures, "libGL");
    // LINK_NAMESPACE(glEnable, "libGL");
    // LINK_NAMESPACE(glDisable, "libGL");
    // LINK_NAMESPACE(glVertexPointer, "libGL");
    // LINK_NAMESPACE(glDrawArrays, "libGL");
    // LINK_NAMESPACE(glMatrixMode, "libGL");
    // LINK_NAMESPACE(glPushMatrix, "libGL");
    // LINK_NAMESPACE(glPopMatrix, "libGL");
    // LINK_NAMESPACE(glLoadIdentity, "libGL");
    // LINK_NAMESPACE(glOrtho, "libGL");
    // LINK_NAMESPACE(glBlendFunc, "libGL");
    LINK_NAMESPACE(glTexParameteri, "libGL");
    LINK_NAMESPACE(glGetIntegerv, "libGL");
    // LINK_NAMESPACE(glGetBooleanv, "libGL");
    // LINK_NAMESPACE(glUseProgram, "libGL");
}

void RenderHUD_GL::size(int& width, int& height)
{
    GLint viewport[4];
    orig::glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}

void RenderHUD_GL::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    static int inited = 0;
    if (inited == 0) {
        init();
        inited = 1;
    }
    
    enterGLRender();

    /* TODO: Manage GL textures!!! */
    static GLuint texture = 0;
    if (texture == 0) {
        orig::glGenTextures(1, &texture);
    }
    orig::glBindTexture(GL_TEXTURE_2D, texture);

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);

    orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    orig::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    orig::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surf->pixels.data());

    orig::glBegin(GL_QUADS);
    {
        orig::glTexCoord2f(0,0); orig::glVertex2f(x, y);
        orig::glTexCoord2f(1,0); orig::glVertex2f(x + surf->w, y);
        orig::glTexCoord2f(1,1); orig::glVertex2f(x + surf->w, y + surf->h);
        orig::glTexCoord2f(0,1); orig::glVertex2f(x, y + surf->h);
    }
    orig::glEnd();

    exitGLRender();
}

}

#endif
