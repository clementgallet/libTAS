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

#include <GL/gl.h>

#include "opengl_helpers.h"
#include "hook.h"

namespace libtas {

GLint oldProgram;
GLboolean oldTex2DEnabled;
GLboolean oldBlendEnabled;
GLint oldBlendSrc;
GLint oldBlendDst;
GLint oldTex;

DEFINE_ORIG_POINTER(glGenTextures);
DEFINE_ORIG_POINTER(glBindTexture);
DEFINE_ORIG_POINTER(glTexImage2D);
DEFINE_ORIG_POINTER(glBegin);
DEFINE_ORIG_POINTER(glEnd);
DEFINE_ORIG_POINTER(glVertex2f);
DEFINE_ORIG_POINTER(glTexCoord2f);
DEFINE_ORIG_POINTER(glDeleteTextures);
DEFINE_ORIG_POINTER(glEnable);
DEFINE_ORIG_POINTER(glDisable);
DEFINE_ORIG_POINTER(glVertexPointer);
DEFINE_ORIG_POINTER(glDrawArrays);
DEFINE_ORIG_POINTER(glMatrixMode);
DEFINE_ORIG_POINTER(glPushMatrix);
DEFINE_ORIG_POINTER(glPopMatrix);
DEFINE_ORIG_POINTER(glLoadIdentity);
DEFINE_ORIG_POINTER(glOrtho);
DEFINE_ORIG_POINTER(glBlendFunc);
DEFINE_ORIG_POINTER(glTexParameteri);
DEFINE_ORIG_POINTER(glGetIntegerv);
DEFINE_ORIG_POINTER(glGetBooleanv);

namespace orig {
    static void (*glUseProgram)(unsigned int program);
}

void enterGLRender(void)
{
    LINK_NAMESPACE(glGenTextures, "libGL");
    LINK_NAMESPACE(glBindTexture, "libGL");
    LINK_NAMESPACE(glTexImage2D, "libGL");
    LINK_NAMESPACE(glBegin, "libGL");
    LINK_NAMESPACE(glEnd, "libGL");
    LINK_NAMESPACE(glVertex2f, "libGL");
    LINK_NAMESPACE(glTexCoord2f, "libGL");
    LINK_NAMESPACE(glDeleteTextures, "libGL");
    LINK_NAMESPACE(glEnable, "libGL");
    LINK_NAMESPACE(glDisable, "libGL");
    LINK_NAMESPACE(glVertexPointer, "libGL");
    LINK_NAMESPACE(glDrawArrays, "libGL");
    LINK_NAMESPACE(glMatrixMode, "libGL");
    LINK_NAMESPACE(glPushMatrix, "libGL");
    LINK_NAMESPACE(glPopMatrix, "libGL");
    LINK_NAMESPACE(glLoadIdentity, "libGL");
    LINK_NAMESPACE(glOrtho, "libGL");
    LINK_NAMESPACE(glBlendFunc, "libGL");
    LINK_NAMESPACE(glTexParameteri, "libGL");
    LINK_NAMESPACE(glGetIntegerv, "libGL");
    LINK_NAMESPACE(glGetBooleanv, "libGL");
    LINK_NAMESPACE(glUseProgram, "libGL");

    orig::glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    orig::glUseProgram(0);

    GLint viewport[4];
    orig::glGetIntegerv(GL_VIEWPORT, viewport);

    orig::glMatrixMode(GL_PROJECTION);
    orig::glPushMatrix();
    orig::glLoadIdentity();
    orig::glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1] + viewport[3], viewport[1], -1, 1);

    orig::glMatrixMode(GL_MODELVIEW);
    orig::glPushMatrix();
    orig::glLoadIdentity();

    orig::glDisable(GL_DEPTH_TEST);

    orig::glGetBooleanv(GL_TEXTURE_2D, &oldTex2DEnabled);

    orig::glEnable(GL_TEXTURE_2D);

    orig::glGetBooleanv(GL_BLEND, &oldBlendEnabled);

    orig::glGetIntegerv(GL_BLEND_SRC_ALPHA, &oldBlendSrc);
    orig::glGetIntegerv(GL_BLEND_DST_ALPHA, &oldBlendDst);

    orig::glEnable(GL_BLEND);
    orig::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Get previous blind texture */
    orig::glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
}

void exitGLRender(void)
{
    if (oldTex != 0) {
        orig::glBindTexture(GL_TEXTURE_2D, oldTex);
    }

    if (! oldBlendEnabled )
        orig::glDisable(GL_BLEND);
    orig::glBlendFunc(oldBlendSrc, oldBlendDst);

    if (! oldTex2DEnabled )
        orig::glDisable(GL_TEXTURE_2D);

    orig::glMatrixMode(GL_PROJECTION);
    orig::glPopMatrix();
    orig::glMatrixMode(GL_MODELVIEW);
    orig::glPopMatrix();

    orig::glUseProgram(oldProgram);
}

}
