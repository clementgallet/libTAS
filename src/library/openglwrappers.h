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
#include <GL/glxext.h>

namespace libtas {

void checkMesa();

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

OVERRIDE GLXContext glXCreateContextAttribsARB (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);

OVERRIDE const GLubyte* glGetString( GLenum name);

/* A lot of openGL draw commands that are NOPed when fastforwarding */

OVERRIDE void glClear(GLbitfield mask);
void myglClear(GLbitfield mask);

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

void myglDrawTransformFeedback (GLenum mode, GLuint id);
void myglDrawTransformFeedbackStream (GLenum mode, GLuint id, GLuint stream);
void myglDrawTransformFeedbackInstanced (GLenum mode, GLuint id, GLsizei instancecount);
void myglDrawTransformFeedbackStreamInstanced (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);

void myglDrawArraysInstancedARB (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
void myglDrawElementsInstancedARB (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);

void myglDrawArraysInstancedEXT (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
void myglDrawElementsInstancedEXT (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
void myglDrawRangeElementsEXT (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);

void myglMultiDrawArraysEXT (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
void myglMultiDrawElementsEXT (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount);

void myglDrawArraysEXT (GLenum mode, GLint first, GLsizei count);

OVERRIDE void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void myglBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

};

#endif
