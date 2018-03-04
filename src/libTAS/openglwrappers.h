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

#ifndef LIBTAS_OPENGLWRAPPERS_H_INCL
#define LIBTAS_OPENGLWRAPPERS_H_INCL

#include "global.h"
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

namespace libtas {

OVERRIDE void(*glXGetProcAddress (const GLubyte *procName))();
OVERRIDE __GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName);
OVERRIDE void* glXGetProcAddressEXT (const GLubyte *procName);

/* Map the GLX context to the Display connection */
OVERRIDE Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx );

/* Swap buffers */
OVERRIDE void glXSwapBuffers( Display *dpy, GLXDrawable drawable );

/* Set the VSync value */
OVERRIDE void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval);
OVERRIDE int glXSwapIntervalSGI (int interval);
OVERRIDE int glXSwapIntervalMESA (unsigned int interval);

OVERRIDE int glXGetSwapIntervalMESA(void);

/* Returns an attribute assoicated with a GLX drawable */
OVERRIDE void glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value);

void myglDrawArrays( GLenum mode, GLint first, GLsizei count );
void myglDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

void myglMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
void myglMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);

void myglDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);

void myglDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
void myglDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
void myglDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
void myglMultiDrawElementsBaseVertex (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);

void myglDrawArraysInstancedBaseInstance (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
void myglDrawElementsInstancedBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
void myglDrawElementsInstancedBaseVertexBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);

};

#endif
