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

#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD_GL.h"
#include "../opengl_helpers.h"
#include "../logging.h"
#include "../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(glGetIntegerv)
DEFINE_ORIG_POINTER(glGenTextures)
DEFINE_ORIG_POINTER(glDeleteTextures)
DEFINE_ORIG_POINTER(glBindTexture)
DEFINE_ORIG_POINTER(glTexParameteri)
DEFINE_ORIG_POINTER(glTexImage2D)
DEFINE_ORIG_POINTER(glBegin)
DEFINE_ORIG_POINTER(glTexCoord2f)
DEFINE_ORIG_POINTER(glVertex2f)
DEFINE_ORIG_POINTER(glEnd)
DEFINE_ORIG_POINTER(glActiveTexture)

GLuint RenderHUD_GL::texture = 0;

RenderHUD_GL::RenderHUD_GL() : RenderHUD()
{
    if (texture == 0) {
        LINK_NAMESPACE(glGenTextures, "libGL");
        LINK_NAMESPACE(glGetIntegerv, "libGL");
        LINK_NAMESPACE(glActiveTexture, "libGL");

        /* Get previous active texture */
        GLint oldActiveTex;
        orig::glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTex);
        orig::glActiveTexture(GL_TEXTURE0);

        orig::glGenTextures(1, &texture);

        /* Restore previous active texture */
        if (oldActiveTex != 0) {
            orig::glActiveTexture(oldActiveTex);
        }
    }
}

RenderHUD_GL::~RenderHUD_GL()
{
    if (texture != 0) {
        LINK_NAMESPACE(glDeleteTextures, "libGL");
        orig::glDeleteTextures(1, &texture);
        texture = 0;
    }
}

void RenderHUD_GL::box(int& x, int& y, int& width, int& height)
{
    GLint viewport[4];
    LINK_NAMESPACE(glGetIntegerv, "libGL");
    orig::glGetIntegerv(GL_VIEWPORT, viewport);
    x = viewport[0];
    y = viewport[1];
    width = viewport[2];
    height = viewport[3];
}

void RenderHUD_GL::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    LINK_NAMESPACE(glBindTexture, "libGL");
    LINK_NAMESPACE(glTexImage2D, "libGL");
    LINK_NAMESPACE(glBegin, "libGL");
    LINK_NAMESPACE(glEnd, "libGL");
    LINK_NAMESPACE(glVertex2f, "libGL");
    LINK_NAMESPACE(glTexCoord2f, "libGL");
    LINK_NAMESPACE(glTexParameteri, "libGL");

    enterGLRender();

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
