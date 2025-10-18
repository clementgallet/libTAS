/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "openglloader.h"

#include "global.h"
#include "GlobalState.h"
#include "logging.h"
#include "frame.h"
#include "screencapture/ScreenCapture.h"

namespace libtas {

#define GLFUNCSKIPDRAW(NAME, DECL, ARGS) \
void gl##NAME DECL\
{\
    LINK_GL_POINTER(NAME);\
    if (GlobalState::isNative()) return glProcs.NAME ARGS;\
    LOGTRACE(LCF_OGL);\
    if (!Global::skipping_draw)\
        return glProcs.NAME ARGS;\
}\
\
void my##gl##NAME DECL\
{\
    if (GlobalState::isNative()) return glProcs.NAME ARGS;\
    LOGTRACE(LCF_OGL);\
    if (!Global::skipping_draw)\
        return glProcs.NAME ARGS ;\
}

GLFUNCSKIPDRAW(Clear, (GLbitfield mask), (mask))
GLFUNCSKIPDRAW(Begin, (GLenum mode), (mode))
GLFUNCSKIPDRAW(End, (), ())
GLFUNCSKIPDRAW(Vertex2d, (GLdouble x, GLdouble y), (x, y))
GLFUNCSKIPDRAW(Vertex2f, (GLfloat x, GLfloat y), (x, y))
GLFUNCSKIPDRAW(Vertex2i, (GLint x, GLint y), (x, y))
GLFUNCSKIPDRAW(Vertex2s, (GLshort x, GLshort y), (x, y))
GLFUNCSKIPDRAW(Vertex3d, (GLdouble x, GLdouble y, GLdouble z), (x, y, z))
GLFUNCSKIPDRAW(Vertex3f, (GLfloat x, GLfloat y, GLfloat z), (x, y, z))
GLFUNCSKIPDRAW(Vertex3i, (GLint x, GLint y, GLint z), (x, y, z))
GLFUNCSKIPDRAW(Vertex3s, (GLshort x, GLshort y, GLshort z), (x, y, z))
GLFUNCSKIPDRAW(Vertex4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w), (x, y, z, w))
GLFUNCSKIPDRAW(Vertex4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w), (x, y, z, w))
GLFUNCSKIPDRAW(Vertex4i, (GLint x, GLint y, GLint z, GLint w), (x, y, z, w))
GLFUNCSKIPDRAW(Vertex4s, (GLshort x, GLshort y, GLshort z, GLshort w), (x, y, z, w))
GLFUNCSKIPDRAW(Vertex2dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(Vertex2fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(Vertex2iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(Vertex2sv, (const GLshort *v), (v))
GLFUNCSKIPDRAW(Vertex3dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(Vertex3fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(Vertex3iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(Vertex3sv, (const GLshort *v), (v))
GLFUNCSKIPDRAW(Vertex4dv, (const GLdouble *v), (v))
GLFUNCSKIPDRAW(Vertex4fv, (const GLfloat *v), (v))
GLFUNCSKIPDRAW(Vertex4iv, (const GLint *v), (v))
GLFUNCSKIPDRAW(Vertex4sv, (const GLshort *v), (v))

GLFUNCSKIPDRAW(DrawArrays, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
GLFUNCSKIPDRAW(DrawElements, (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices), (mode, count, type, indices))
GLFUNCSKIPDRAW(MultiDrawArrays, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount), (mode, first, count, drawcount))
GLFUNCSKIPDRAW(MultiDrawElements, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount), (mode, count, type, indices, drawcount))
GLFUNCSKIPDRAW(DrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices), (mode, start, end, count, type, indices))
GLFUNCSKIPDRAW(DrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, count, type, indices, basevertex))
GLFUNCSKIPDRAW(DrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, start, end, count, type, indices, basevertex))
GLFUNCSKIPDRAW(DrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex), (mode, count, type, indices, instancecount, basevertex))
GLFUNCSKIPDRAW(MultiDrawElementsBaseVertex, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex), (mode, count, type, indices, drawcount, basevertex))

#ifdef GL_VERSION_4_0
GLFUNCSKIPDRAW(DrawTransformFeedback, (GLenum mode, GLuint id), (mode, id))
GLFUNCSKIPDRAW(DrawTransformFeedbackStream, (GLenum mode, GLuint id, GLuint stream), (mode, id, stream))
#endif

#ifdef GL_VERSION_4_2
GLFUNCSKIPDRAW(DrawArraysInstancedBaseInstance, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance), (mode, first, count, instancecount, baseinstance))
GLFUNCSKIPDRAW(DrawElementsInstancedBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance), (mode, count, type, indices, instancecount, baseinstance))
GLFUNCSKIPDRAW(DrawElementsInstancedBaseVertexBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance), (mode, count, type, indices, instancecount, basevertex, baseinstance))
GLFUNCSKIPDRAW(DrawTransformFeedbackInstanced, (GLenum mode, GLuint id, GLsizei instancecount), (mode, id, instancecount))
GLFUNCSKIPDRAW(DrawTransformFeedbackStreamInstanced, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount), (mode, id, stream, instancecount))
#endif

#ifdef GL_ARB_draw_instanced
GLFUNCSKIPDRAW(DrawArraysInstancedARB, (GLenum mode, GLint first, GLsizei count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(DrawElementsInstancedARB, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif
    
#ifdef GL_EXT_draw_instanced
GLFUNCSKIPDRAW(DrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount), (mode, start, count, primcount))
GLFUNCSKIPDRAW(DrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif

#ifdef GL_EXT_draw_range_elements
GLFUNCSKIPDRAW(DrawRangeElementsEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices), (mode, start, end, count, type, indices))
#endif

#ifdef GL_EXT_multi_draw_arrays
GLFUNCSKIPDRAW(MultiDrawArraysEXT, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(MultiDrawElementsEXT, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount), (mode, count, type, indices, primcount))
#endif

#ifdef GL_EXT_vertex_array
GLFUNCSKIPDRAW(DrawArraysEXT, (GLenum mode, GLint first, GLsizei count), (mode, first, count))
#endif

void myglBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    LOGTRACE(LCF_OGL);
    if (!Global::skipping_draw)
        return glProcs.BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    LINK_GL_POINTER(TexParameterf);
    return myglTexParameterf(target, pname, param);
}

void myglTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    LOGTRACE(LCF_OGL);
    if (Global::shared_config.opengl_performance) {
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
    return glProcs.TexParameterf(target, pname, param);
}

void glTexParameteri(GLenum target, GLenum pname, GLfloat param)
{
    LINK_GL_POINTER(TexParameteri);
    return myglTexParameteri(target, pname, param);
}

void myglTexParameteri(GLenum target, GLenum pname, GLint param)
{
    LOGTRACE(LCF_OGL);
    if (Global::shared_config.opengl_performance) {
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
    return glProcs.TexParameteri(target, pname, param);
}

void glEnable(GLenum cap)
{
    LINK_GL_POINTER(Enable);
    return myglEnable(cap);
}

void myglEnable(GLenum cap)
{
    if (GlobalState::isNative()) return glProcs.Enable(cap);
    LOGTRACE(LCF_OGL);
    if (Global::shared_config.opengl_performance) {
        switch (cap) {
            case GL_MULTISAMPLE:
            case GL_DITHER:
            case GL_TEXTURE_CUBE_MAP_SEAMLESS:
            // case GL_BLEND:
                return;
        }
    }
    return glProcs.Enable(cap);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    LINK_GL_POINTER(Viewport);
    return myglViewport(x, y, width, height);
}

void myglViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (GlobalState::isNative()) return glProcs.Viewport(x, y, width, height);
    LOG(LL_TRACE, LCF_OGL, "glViewport called with %d : %d", width, height);
    return glProcs.Viewport(x, y, width, height);
}

void glGetIntegerv(GLenum pname, GLint *data)
{
    LINK_GL_POINTER(GetIntegerv);
    return myglGetIntegerv(pname, data);
}

void myglGetIntegerv(GLenum pname, GLint *data)
{
    if (GlobalState::isNative()) return glProcs.GetIntegerv(pname, data);
    LOGTRACE(LCF_OGL);
    if (Global::shared_config.opengl_soft)
    {
        // Unity 4.x uses GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX to determine if there is enough memory for a texture
        // However, llvmpipe is bugged and will return an uninitialized variable, i.e. non-deterministic
        // See https://gitlab.freedesktop.org/mesa/mesa/-/issues/14064
        if (pname == GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX)
        {
            *data = 65536;
            return;
        }
    }

    return glProcs.GetIntegerv(pname, data);
}

}
