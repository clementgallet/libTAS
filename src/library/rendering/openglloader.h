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

#ifndef LIBTAS_OPENGLLOADER_H_INCL
#define LIBTAS_OPENGLLOADER_H_INCL

#include "hook.h"

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#ifdef __unix__
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#define DEFINE_GL_POINTER(FUNC) \
    decltype(&gl##FUNC) FUNC;

namespace libtas {

/* gl3w internal state */
struct GLProcs {
    
    DEFINE_GL_POINTER(XMakeCurrent)
    DEFINE_GL_POINTER(XMakeContextCurrent)
    DEFINE_GL_POINTER(XSwapBuffers)
    DEFINE_GL_POINTER(XSwapIntervalEXT)
    DEFINE_GL_POINTER(XSwapIntervalSGI)
    DEFINE_GL_POINTER(XSwapIntervalMESA)
    DEFINE_GL_POINTER(XGetSwapIntervalMESA)
    DEFINE_GL_POINTER(XQueryExtensionsString)
    DEFINE_GL_POINTER(XQueryDrawable)
    DEFINE_GL_POINTER(XCreateContextAttribsARB)
    DEFINE_GL_POINTER(XDestroyContext)

    DEFINE_GL_POINTER(GetString)
    DEFINE_GL_POINTER(BlitFramebuffer)
    DEFINE_GL_POINTER(TexParameterf)
    DEFINE_GL_POINTER(TexParameteri)
    DEFINE_GL_POINTER(Enable)
    DEFINE_GL_POINTER(ReadPixels)
    DEFINE_GL_POINTER(GenFramebuffers)
    DEFINE_GL_POINTER(BindFramebuffer)
    DEFINE_GL_POINTER(DeleteFramebuffers)
    DEFINE_GL_POINTER(GenRenderbuffers)
    DEFINE_GL_POINTER(BindRenderbuffer)
    DEFINE_GL_POINTER(DeleteRenderbuffers)
    DEFINE_GL_POINTER(RenderbufferStorage)
    DEFINE_GL_POINTER(FramebufferRenderbuffer)
    DEFINE_GL_POINTER(Disable)
    DEFINE_GL_POINTER(IsEnabled)
    DEFINE_GL_POINTER(GetIntegerv)
    DEFINE_GL_POINTER(GetError)
    DEFINE_GL_POINTER(GenTextures)
    DEFINE_GL_POINTER(DeleteTextures)
    DEFINE_GL_POINTER(BindTexture)
    DEFINE_GL_POINTER(BindSampler)
    DEFINE_GL_POINTER(TexImage2D)
    DEFINE_GL_POINTER(ActiveTexture)
    DEFINE_GL_POINTER(FramebufferTexture2D)
    DEFINE_GL_POINTER(UseProgram)
    DEFINE_GL_POINTER(PixelStorei)
    DEFINE_GL_POINTER(GenBuffers)
    DEFINE_GL_POINTER(GenVertexArrays)
    DEFINE_GL_POINTER(BindVertexArray)
    DEFINE_GL_POINTER(BindBuffer)
    DEFINE_GL_POINTER(BufferData)
    DEFINE_GL_POINTER(VertexAttribPointer)
    DEFINE_GL_POINTER(EnableVertexAttribArray)
    DEFINE_GL_POINTER(CreateShader)
    DEFINE_GL_POINTER(ShaderSource)
    DEFINE_GL_POINTER(CompileShader)
    DEFINE_GL_POINTER(GetShaderiv)
    DEFINE_GL_POINTER(GetShaderInfoLog)
    DEFINE_GL_POINTER(CreateProgram)
    DEFINE_GL_POINTER(AttachShader)
    DEFINE_GL_POINTER(LinkProgram)
    DEFINE_GL_POINTER(GetProgramiv)
    DEFINE_GL_POINTER(GetProgramInfoLog)
    DEFINE_GL_POINTER(DetachShader)
    DEFINE_GL_POINTER(DeleteShader)
    DEFINE_GL_POINTER(BlendFunc)
    DEFINE_GL_POINTER(DeleteBuffers)
    DEFINE_GL_POINTER(DeleteVertexArrays)
    DEFINE_GL_POINTER(DeleteProgram)
    DEFINE_GL_POINTER(Viewport)
    DEFINE_GL_POINTER(Clear)
    DEFINE_GL_POINTER(Begin)
    DEFINE_GL_POINTER(End)
    DEFINE_GL_POINTER(Vertex2d)
    DEFINE_GL_POINTER(Vertex2f)
    DEFINE_GL_POINTER(Vertex2i)
    DEFINE_GL_POINTER(Vertex2s)
    DEFINE_GL_POINTER(Vertex3d)
    DEFINE_GL_POINTER(Vertex3f)
    DEFINE_GL_POINTER(Vertex3i)
    DEFINE_GL_POINTER(Vertex3s)
    DEFINE_GL_POINTER(Vertex4d)
    DEFINE_GL_POINTER(Vertex4f)
    DEFINE_GL_POINTER(Vertex4i)
    DEFINE_GL_POINTER(Vertex4s)
    DEFINE_GL_POINTER(Vertex2dv)
    DEFINE_GL_POINTER(Vertex2fv)
    DEFINE_GL_POINTER(Vertex2iv)
    DEFINE_GL_POINTER(Vertex2sv)
    DEFINE_GL_POINTER(Vertex3dv)
    DEFINE_GL_POINTER(Vertex3fv)
    DEFINE_GL_POINTER(Vertex3iv)
    DEFINE_GL_POINTER(Vertex3sv)
    DEFINE_GL_POINTER(Vertex4dv)
    DEFINE_GL_POINTER(Vertex4fv)
    DEFINE_GL_POINTER(Vertex4iv)
    DEFINE_GL_POINTER(Vertex4sv)

    DEFINE_GL_POINTER(DrawArrays)
    DEFINE_GL_POINTER(DrawElements)
    DEFINE_GL_POINTER(MultiDrawArrays)
    DEFINE_GL_POINTER(MultiDrawElements)
    DEFINE_GL_POINTER(DrawRangeElements)
    DEFINE_GL_POINTER(DrawElementsBaseVertex)
    DEFINE_GL_POINTER(DrawRangeElementsBaseVertex)
    DEFINE_GL_POINTER(DrawElementsInstancedBaseVertex)
    DEFINE_GL_POINTER(MultiDrawElementsBaseVertex)

    #ifdef GL_VERSION_4_0
    DEFINE_GL_POINTER(DrawTransformFeedback)
    DEFINE_GL_POINTER(DrawTransformFeedbackStream)
    #endif

    #ifdef GL_VERSION_4_2
    DEFINE_GL_POINTER(DrawArraysInstancedBaseInstance)
    DEFINE_GL_POINTER(DrawElementsInstancedBaseInstance)
    DEFINE_GL_POINTER(DrawElementsInstancedBaseVertexBaseInstance)
    DEFINE_GL_POINTER(DrawTransformFeedbackInstanced)
    DEFINE_GL_POINTER(DrawTransformFeedbackStreamInstanced)
    #endif

    #ifdef GL_ARB_draw_instanced
    DEFINE_GL_POINTER(DrawArraysInstancedARB)
    DEFINE_GL_POINTER(DrawElementsInstancedARB)
    #endif

    #ifdef GL_EXT_draw_instanced
    DEFINE_GL_POINTER(DrawArraysInstancedEXT)
    DEFINE_GL_POINTER(DrawElementsInstancedEXT)
    #endif

    #ifdef GL_EXT_draw_range_elements
    DEFINE_GL_POINTER(DrawRangeElementsEXT)
    #endif
    #ifdef GL_EXT_multi_draw_arrays
    DEFINE_GL_POINTER(MultiDrawArraysEXT)
    DEFINE_GL_POINTER(MultiDrawElementsEXT)
    #endif
    #ifdef GL_EXT_vertex_array
    DEFINE_GL_POINTER(DrawArraysEXT)
    #endif
};

typedef void (*GLProc)(void);
typedef GLProc (*GLGetProcAddressProc)(const char *proc);

void gl_load_procs(GLGetProcAddressProc proc);

extern struct GLProcs glProcs;

#define LINK_GL_POINTER(FUNC) \
    if (!glProcs.FUNC) \
        link_function((void**)&glProcs.FUNC, "gl" #FUNC, "libGL.so");

}

#endif
