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

#include "openglwrappers.h"
#include "hook.h"
#include "logging.h"
// #include "renderhud/RenderHUD_GL.h"
// #include "ScreenCapture.h"
#include "frame.h"
// #include "xlib/xwindows.h" // x11::gameXWindows

// #include <string.h>

namespace libtas {

DEFINE_ORIG_POINTER(glGetString)
DEFINE_ORIG_POINTER(glBlitFramebuffer)
DEFINE_ORIG_POINTER(glTexParameterf)
DEFINE_ORIG_POINTER(glTexParameteri)
DEFINE_ORIG_POINTER(glEnable)
DEFINE_ORIG_POINTER(glReadPixels)
DEFINE_ORIG_POINTER(glGenFramebuffers)
DEFINE_ORIG_POINTER(glBindFramebuffer)
DEFINE_ORIG_POINTER(glDeleteFramebuffers)
DEFINE_ORIG_POINTER(glGenRenderbuffers)
DEFINE_ORIG_POINTER(glBindRenderbuffer)
DEFINE_ORIG_POINTER(glDeleteRenderbuffers)
DEFINE_ORIG_POINTER(glRenderbufferStorage)
DEFINE_ORIG_POINTER(glFramebufferRenderbuffer)
DEFINE_ORIG_POINTER(glDisable)
DEFINE_ORIG_POINTER(glIsEnabled)
DEFINE_ORIG_POINTER(glGetIntegerv)
DEFINE_ORIG_POINTER(glGetError)
DEFINE_ORIG_POINTER(glGenTextures)
DEFINE_ORIG_POINTER(glDeleteTextures)
DEFINE_ORIG_POINTER(glBindTexture)
DEFINE_ORIG_POINTER(glBindSampler)
DEFINE_ORIG_POINTER(glTexImage2D)
DEFINE_ORIG_POINTER(glActiveTexture)
DEFINE_ORIG_POINTER(glFramebufferTexture2D)
DEFINE_ORIG_POINTER(glUseProgram)
DEFINE_ORIG_POINTER(glPixelStorei)

DEFINE_ORIG_POINTER(glGenBuffers)
DEFINE_ORIG_POINTER(glGenVertexArrays)
DEFINE_ORIG_POINTER(glBindVertexArray)
DEFINE_ORIG_POINTER(glBindBuffer)
DEFINE_ORIG_POINTER(glBufferData)
DEFINE_ORIG_POINTER(glVertexAttribPointer)
DEFINE_ORIG_POINTER(glEnableVertexAttribArray)
DEFINE_ORIG_POINTER(glCreateShader)
DEFINE_ORIG_POINTER(glShaderSource)
DEFINE_ORIG_POINTER(glCompileShader)
DEFINE_ORIG_POINTER(glGetShaderiv)
DEFINE_ORIG_POINTER(glGetShaderInfoLog)
DEFINE_ORIG_POINTER(glCreateProgram)
DEFINE_ORIG_POINTER(glAttachShader)
DEFINE_ORIG_POINTER(glLinkProgram)
DEFINE_ORIG_POINTER(glGetProgramiv)
DEFINE_ORIG_POINTER(glGetProgramInfoLog)
DEFINE_ORIG_POINTER(glDetachShader)
DEFINE_ORIG_POINTER(glDeleteShader)
DEFINE_ORIG_POINTER(glBlendFunc)
DEFINE_ORIG_POINTER(glDeleteBuffers)
DEFINE_ORIG_POINTER(glDeleteVertexArrays)
DEFINE_ORIG_POINTER(glDeleteProgram)

#define GLFUNCSKIPDRAW(NAME, DECL, ARGS) \
DEFINE_ORIG_POINTER(NAME)\
void NAME DECL\
{\
    DEBUGLOGCALL(LCF_OGL);\
    LINK_NAMESPACE(NAME, "GL");\
    if (!skipping_draw)\
        return orig::NAME ARGS;\
}\
\
void my##NAME DECL\
{\
    DEBUGLOGCALL(LCF_OGL);\
    if (!skipping_draw)\
        return orig::NAME ARGS ;\
}

GLFUNCSKIPDRAW(glClear, (GLbitfield mask), (mask))
GLFUNCSKIPDRAW(glBegin, (GLenum mode), (mode))
GLFUNCSKIPDRAW(glEnd, (), ())
GLFUNCSKIPDRAW(glVertex2d, (GLdouble x, GLdouble y), (x, y))
GLFUNCSKIPDRAW(glVertex2f, (GLfloat x, GLfloat y), (x, y))
GLFUNCSKIPDRAW(glVertex2i, (GLint x, GLint y), (x, y))
GLFUNCSKIPDRAW(glVertex2s, (GLshort x, GLshort y), (x, y))
GLFUNCSKIPDRAW(glVertex3d, (GLdouble x, GLdouble y, GLdouble z), (x, y, z))
GLFUNCSKIPDRAW(glVertex3f, (GLfloat x, GLfloat y, GLfloat z), (x, y, z))
GLFUNCSKIPDRAW(glVertex3i, (GLint x, GLint y, GLint z), (x, y, z))
GLFUNCSKIPDRAW(glVertex3s, (GLshort x, GLshort y, GLshort z), (x, y, z))
GLFUNCSKIPDRAW(glVertex4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w), (x, y, z, w))
GLFUNCSKIPDRAW(glVertex4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w), (x, y, z, w))
GLFUNCSKIPDRAW(glVertex4i, (GLint x, GLint y, GLint z, GLint w), (x, y, z, w))
GLFUNCSKIPDRAW(glVertex4s, (GLshort x, GLshort y, GLshort z, GLshort w), (x, y, z, w))
GLFUNCSKIPDRAW(glVertex2dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(glVertex2fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(glVertex2iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(glVertex2sv, (const GLshort *v), (v))
GLFUNCSKIPDRAW(glVertex3dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(glVertex3fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(glVertex3iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(glVertex3sv, (const GLshort *v), (v))
GLFUNCSKIPDRAW(glVertex4dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(glVertex4fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(glVertex4iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(glVertex4sv, (const GLshort *v), (v))

GLFUNCSKIPDRAW(glDrawArrays, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
GLFUNCSKIPDRAW(glDrawElements, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices), (mode, count, type, indices))
GLFUNCSKIPDRAW(glMultiDrawArrays, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount), (mode, first, count, drawcount))
GLFUNCSKIPDRAW(glMultiDrawElements, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount), (mode, count, type, indices, drawcount))
GLFUNCSKIPDRAW(glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices), (mode, start, end, count, type, indices))
GLFUNCSKIPDRAW(glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, count, type, indices, basevertex))
GLFUNCSKIPDRAW(glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, start, end, count, type, indices, basevertex))
GLFUNCSKIPDRAW(glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex), (mode, count, type, indices, instancecount, basevertex))
GLFUNCSKIPDRAW(glMultiDrawElementsBaseVertex, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex), (mode, count, type, indices, drawcount, basevertex))

#ifdef GL_VERSION_4_0
GLFUNCSKIPDRAW(glDrawTransformFeedback, (GLenum mode, GLuint id), (mode, id))
GLFUNCSKIPDRAW(glDrawTransformFeedbackStream, (GLenum mode, GLuint id, GLuint stream), (mode, id, stream))
#endif

#ifdef GL_VERSION_4_2
GLFUNCSKIPDRAW(glDrawArraysInstancedBaseInstance, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance), (mode, first, count, instancecount, baseinstance))
GLFUNCSKIPDRAW(glDrawElementsInstancedBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance), (mode, count, type, indices, instancecount, baseinstance))
GLFUNCSKIPDRAW(glDrawElementsInstancedBaseVertexBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance), (mode, count, type, indices, instancecount, basevertex, baseinstance))
GLFUNCSKIPDRAW(glDrawTransformFeedbackInstanced, (GLenum mode, GLuint id, GLsizei instancecount), (mode, id, instancecount))
GLFUNCSKIPDRAW(glDrawTransformFeedbackStreamInstanced, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount), (mode, id, stream, instancecount))
#endif

#ifdef GL_ARB_draw_instanced
GLFUNCSKIPDRAW(glDrawArraysInstancedARB, (GLenum mode, GLint first, GLsizei count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(glDrawElementsInstancedARB, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif
    
#ifdef GL_EXT_draw_instanced
GLFUNCSKIPDRAW(glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount), (mode, start, count, primcount))
GLFUNCSKIPDRAW(glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif

#ifdef GL_EXT_draw_range_elements
GLFUNCSKIPDRAW(glDrawRangeElementsEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices), (mode, start, end, count, type, indices))
#endif

#ifdef GL_EXT_multi_draw_arrays
GLFUNCSKIPDRAW(glMultiDrawArraysEXT, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(glMultiDrawElementsEXT, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif

#ifdef GL_EXT_vertex_array
GLFUNCSKIPDRAW(glDrawArraysEXT, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
#endif

// void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
// {
//     LINK_NAMESPACE(glBlitFramebuffer, "GL");
//     return myglBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
// }

void myglBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    LINK_NAMESPACE(glTexParameterf, "GL");
    return myglTexParameterf(target, pname, param);
}

void myglTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    DEBUGLOGCALL(LCF_OGL);
    if (shared_config.opengl_performance) {
        switch (pname) {
            case GL_TEXTURE_MIN_FILTER:
                if (param == GL_NEAREST) {}
                else if (param == GL_LINEAR)
                    param = GL_NEAREST;
                else
                    param = GL_NEAREST_MIPMAP_NEAREST;
                break;
            case GL_TEXTURE_MAG_FILTER:
                param = GL_NEAREST;
                break;
            case GL_TEXTURE_MAX_ANISOTROPY_EXT:
                param = 1;
                break;
            case GL_TEXTURE_LOD_BIAS:
                param = 3;
                break;
        }
    }
    return orig::glTexParameterf(target, pname, param);
}

void glTexParameteri(GLenum target, GLenum pname, GLfloat param)
{
    LINK_NAMESPACE(glTexParameteri, "GL");
    return myglTexParameteri(target, pname, param);
}

void myglTexParameteri(GLenum target, GLenum pname, GLint param)
{
    DEBUGLOGCALL(LCF_OGL);
    if (shared_config.opengl_performance) {
        switch (pname) {
            case GL_TEXTURE_MIN_FILTER:
                if (param == GL_NEAREST) {}
                else if (param == GL_LINEAR)
                    param = GL_NEAREST;
                else
                    param = GL_NEAREST_MIPMAP_NEAREST;
                break;
            case GL_TEXTURE_MAG_FILTER:
                param = GL_NEAREST;
                break;
            case GL_TEXTURE_MAX_ANISOTROPY_EXT:
                param = 1;
                break;
            case GL_TEXTURE_LOD_BIAS:
                param = 3;
                break;
        }
    }
    return orig::glTexParameteri(target, pname, param);
}

void glEnable(GLenum cap)
{
    LINK_NAMESPACE(glEnable, "GL");
    return myglEnable(cap);
}

void myglEnable(GLenum cap)
{
    DEBUGLOGCALL(LCF_OGL);
    if (shared_config.opengl_performance) {
        switch (cap) {
            case GL_MULTISAMPLE:
            case GL_DITHER:
            case GL_TEXTURE_CUBE_MAP_SEAMLESS:
            // case GL_BLEND:
                return;
        }
    }
    return orig::glEnable(cap);
}

}
