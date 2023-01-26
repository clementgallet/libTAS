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

#ifndef LIBTAS_OPENGLWRAPPERS_H_INCL
#define LIBTAS_OPENGLWRAPPERS_H_INCL

#include "hook.h"
#define GL_GLEXT_PROTOTYPES
#ifdef __unix__
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

namespace libtas {

OVERRIDE const GLubyte* glGetString( GLenum name);

/* A lot of openGL draw commands that are NOPed when fastforwarding */

OVERRIDE void glClear(GLbitfield mask);

OVERRIDE void glBegin(GLenum mode);
OVERRIDE void glEnd(void);

OVERRIDE void glVertex2d( GLdouble x, GLdouble y );
OVERRIDE void glVertex2f( GLfloat x, GLfloat y );
OVERRIDE void glVertex2i( GLint x, GLint y );
OVERRIDE void glVertex2s( GLshort x, GLshort y );

OVERRIDE void glVertex3d( GLdouble x, GLdouble y, GLdouble z );
OVERRIDE void glVertex3f( GLfloat x, GLfloat y, GLfloat z );
OVERRIDE void glVertex3i( GLint x, GLint y, GLint z );
OVERRIDE void glVertex3s( GLshort x, GLshort y, GLshort z );

OVERRIDE void glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
OVERRIDE void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
OVERRIDE void glVertex4i( GLint x, GLint y, GLint z, GLint w );
OVERRIDE void glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w );

OVERRIDE void glVertex2dv( const GLdouble *v );
OVERRIDE void glVertex2fv( const GLfloat *v );
OVERRIDE void glVertex2iv( const GLint *v );
OVERRIDE void glVertex2sv( const GLshort *v );

OVERRIDE void glVertex3dv( const GLdouble *v );
OVERRIDE void glVertex3fv( const GLfloat *v );
OVERRIDE void glVertex3iv( const GLint *v );
OVERRIDE void glVertex3sv( const GLshort *v );

OVERRIDE void glVertex4dv( const GLdouble *v );
OVERRIDE void glVertex4fv( const GLfloat *v );
OVERRIDE void glVertex4iv( const GLint *v );
OVERRIDE void glVertex4sv( const GLshort *v );


OVERRIDE void glDrawArrays( GLenum mode, GLint first, GLsizei count );
OVERRIDE void glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

// OVERRIDE void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void myglBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

/* Some functions to alter for performance boost */

OVERRIDE void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void myglTexParameterf(GLenum target, GLenum pname, GLfloat param);
OVERRIDE void glTexParameteri(GLenum target, GLenum pname, GLint param);
void myglTexParameteri(GLenum target, GLenum pname, GLint param);
// void myglTexParameterfv(GLenum target, GLenum pname, const GLfloat * params);
// void myglTexParameteriv(GLenum target, GLenum pname, const GLint * params);
// void myglTexParameterIiv(GLenum target, GLenum pname, const GLint * params);
// void myglTexParameterIuiv(GLenum target, GLenum pname, const GLuint * params);

OVERRIDE void glEnable(GLenum cap);
void myglEnable(GLenum cap);
// OVERRIDE void glDisable(GLenum cap);
// void myglDisable(GLenum cap);

/* Declare myfunc functions */ 
#define DECLARE_MY_FUNC(FUNC) \
decltype(FUNC) my##FUNC;

DECLARE_MY_FUNC(glClear)
DECLARE_MY_FUNC(glBegin)
DECLARE_MY_FUNC(glEnd)
DECLARE_MY_FUNC(glVertex2d)
DECLARE_MY_FUNC(glVertex2f)
DECLARE_MY_FUNC(glVertex2i)
DECLARE_MY_FUNC(glVertex2s)
DECLARE_MY_FUNC(glVertex3d)
DECLARE_MY_FUNC(glVertex3f)
DECLARE_MY_FUNC(glVertex3i)
DECLARE_MY_FUNC(glVertex3s)
DECLARE_MY_FUNC(glVertex4d)
DECLARE_MY_FUNC(glVertex4f)
DECLARE_MY_FUNC(glVertex4i)
DECLARE_MY_FUNC(glVertex4s)
DECLARE_MY_FUNC(glVertex2dv)
DECLARE_MY_FUNC(glVertex2fv)
DECLARE_MY_FUNC(glVertex2iv)
DECLARE_MY_FUNC(glVertex2sv)
DECLARE_MY_FUNC(glVertex3dv)
DECLARE_MY_FUNC(glVertex3fv)
DECLARE_MY_FUNC(glVertex3iv)
DECLARE_MY_FUNC(glVertex3sv)
DECLARE_MY_FUNC(glVertex4dv)
DECLARE_MY_FUNC(glVertex4fv)
DECLARE_MY_FUNC(glVertex4iv)
DECLARE_MY_FUNC(glVertex4sv)

DECLARE_MY_FUNC(glDrawArrays)
DECLARE_MY_FUNC(glDrawElements)
DECLARE_MY_FUNC(glMultiDrawArrays)
DECLARE_MY_FUNC(glMultiDrawElements)
DECLARE_MY_FUNC(glDrawRangeElements)
DECLARE_MY_FUNC(glDrawElementsBaseVertex)
DECLARE_MY_FUNC(glDrawRangeElementsBaseVertex)
DECLARE_MY_FUNC(glDrawElementsInstancedBaseVertex)
DECLARE_MY_FUNC(glMultiDrawElementsBaseVertex)

#ifdef GL_VERSION_4_0
DECLARE_MY_FUNC(glDrawTransformFeedback)
DECLARE_MY_FUNC(glDrawTransformFeedbackStream)
#endif

#ifdef GL_VERSION_4_2
DECLARE_MY_FUNC(glDrawArraysInstancedBaseInstance)
DECLARE_MY_FUNC(glDrawElementsInstancedBaseInstance)
DECLARE_MY_FUNC(glDrawElementsInstancedBaseVertexBaseInstance)
DECLARE_MY_FUNC(glDrawTransformFeedbackInstanced)
DECLARE_MY_FUNC(glDrawTransformFeedbackStreamInstanced)
#endif

#ifdef GL_ARB_draw_instanced
DECLARE_MY_FUNC(glDrawArraysInstancedARB)
DECLARE_MY_FUNC(glDrawElementsInstancedARB)
#endif

#ifdef GL_EXT_draw_instanced
DECLARE_MY_FUNC(glDrawArraysInstancedEXT)
DECLARE_MY_FUNC(glDrawElementsInstancedEXT)
#endif

#ifdef GL_EXT_draw_range_elements
DECLARE_MY_FUNC(glDrawRangeElementsEXT)
#endif
#ifdef GL_EXT_multi_draw_arrays
DECLARE_MY_FUNC(glMultiDrawArraysEXT)
DECLARE_MY_FUNC(glMultiDrawElementsEXT)
#endif
#ifdef GL_EXT_vertex_array
DECLARE_MY_FUNC(glDrawArraysEXT)
#endif

}

#endif
