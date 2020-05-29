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

OVERRIDE void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
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

};

#endif
