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

#include "openglloader.h"

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

namespace libtas {

struct GLProcs glProcs;

#define GET_GL_POINTER(FUNC) \
    glProcs.FUNC = reinterpret_cast<decltype(&gl##FUNC)>(proc("gl" #FUNC)); \
    if (!glProcs.FUNC) \
        link_function((void**)&glProcs.FUNC, "gl" #FUNC, "libGL.so");

void gl_load_procs(GLGetProcAddressProc proc)
{
    static bool procs_loaded = false;
    if (procs_loaded)
        return;
    
    GET_GL_POINTER(XMakeCurrent)
    GET_GL_POINTER(XMakeContextCurrent)
    GET_GL_POINTER(XSwapBuffers)
    GET_GL_POINTER(XSwapIntervalEXT)
    GET_GL_POINTER(XSwapIntervalSGI)
    GET_GL_POINTER(XSwapIntervalMESA)
    GET_GL_POINTER(XGetSwapIntervalMESA)
    GET_GL_POINTER(XQueryExtensionsString)
    GET_GL_POINTER(XQueryDrawable)
    GET_GL_POINTER(XCreateContextAttribsARB)
    GET_GL_POINTER(XDestroyContext)

    GET_GL_POINTER(GetString)
    GET_GL_POINTER(BlitFramebuffer)
    GET_GL_POINTER(TexParameterf)
    GET_GL_POINTER(TexParameteri)
    GET_GL_POINTER(Enable)
    GET_GL_POINTER(ReadPixels)
    GET_GL_POINTER(GenFramebuffers)
    GET_GL_POINTER(BindFramebuffer)
    GET_GL_POINTER(DeleteFramebuffers)
    GET_GL_POINTER(GenRenderbuffers)
    GET_GL_POINTER(BindRenderbuffer)
    GET_GL_POINTER(DeleteRenderbuffers)
    GET_GL_POINTER(RenderbufferStorage)
    GET_GL_POINTER(FramebufferRenderbuffer)
    GET_GL_POINTER(Disable)
    GET_GL_POINTER(IsEnabled)
    GET_GL_POINTER(GetIntegerv)
    GET_GL_POINTER(GetError)
    GET_GL_POINTER(GenTextures)
    GET_GL_POINTER(DeleteTextures)
    GET_GL_POINTER(BindTexture)
    GET_GL_POINTER(BindSampler)
    GET_GL_POINTER(TexImage2D)
    GET_GL_POINTER(ActiveTexture)
    GET_GL_POINTER(FramebufferTexture2D)
    GET_GL_POINTER(UseProgram)
    GET_GL_POINTER(PixelStorei)
    GET_GL_POINTER(GenBuffers)
    GET_GL_POINTER(GenVertexArrays)
    GET_GL_POINTER(BindVertexArray)
    GET_GL_POINTER(BindBuffer)
    GET_GL_POINTER(BufferData)
    GET_GL_POINTER(VertexAttribPointer)
    GET_GL_POINTER(EnableVertexAttribArray)
    GET_GL_POINTER(CreateShader)
    GET_GL_POINTER(ShaderSource)
    GET_GL_POINTER(CompileShader)
    GET_GL_POINTER(GetShaderiv)
    GET_GL_POINTER(GetShaderInfoLog)
    GET_GL_POINTER(CreateProgram)
    GET_GL_POINTER(AttachShader)
    GET_GL_POINTER(LinkProgram)
    GET_GL_POINTER(GetProgramiv)
    GET_GL_POINTER(GetProgramInfoLog)
    GET_GL_POINTER(DetachShader)
    GET_GL_POINTER(DeleteShader)
    GET_GL_POINTER(BlendFunc)
    GET_GL_POINTER(DeleteBuffers)
    GET_GL_POINTER(DeleteVertexArrays)
    GET_GL_POINTER(DeleteProgram)
    GET_GL_POINTER(Viewport)
    GET_GL_POINTER(Clear)
    GET_GL_POINTER(Begin)
    GET_GL_POINTER(End)
    GET_GL_POINTER(Vertex2d)
    GET_GL_POINTER(Vertex2f)
    GET_GL_POINTER(Vertex2i)
    GET_GL_POINTER(Vertex2s)
    GET_GL_POINTER(Vertex3d)
    GET_GL_POINTER(Vertex3f)
    GET_GL_POINTER(Vertex3i)
    GET_GL_POINTER(Vertex3s)
    GET_GL_POINTER(Vertex4d)
    GET_GL_POINTER(Vertex4f)
    GET_GL_POINTER(Vertex4i)
    GET_GL_POINTER(Vertex4s)
    GET_GL_POINTER(Vertex2dv)
    GET_GL_POINTER(Vertex2fv)
    GET_GL_POINTER(Vertex2iv)
    GET_GL_POINTER(Vertex2sv)
    GET_GL_POINTER(Vertex3dv)
    GET_GL_POINTER(Vertex3fv)
    GET_GL_POINTER(Vertex3iv)
    GET_GL_POINTER(Vertex3sv)
    GET_GL_POINTER(Vertex4dv)
    GET_GL_POINTER(Vertex4fv)
    GET_GL_POINTER(Vertex4iv)
    GET_GL_POINTER(Vertex4sv)

    GET_GL_POINTER(DrawArrays)
    GET_GL_POINTER(DrawElements)
    GET_GL_POINTER(MultiDrawArrays)
    GET_GL_POINTER(MultiDrawElements)
    GET_GL_POINTER(DrawRangeElements)
    GET_GL_POINTER(DrawElementsBaseVertex)
    GET_GL_POINTER(DrawRangeElementsBaseVertex)
    GET_GL_POINTER(DrawElementsInstancedBaseVertex)
    GET_GL_POINTER(MultiDrawElementsBaseVertex)

    #ifdef GL_VERSION_4_0
    GET_GL_POINTER(DrawTransformFeedback)
    GET_GL_POINTER(DrawTransformFeedbackStream)
    #endif

    #ifdef GL_VERSION_4_2
    GET_GL_POINTER(DrawArraysInstancedBaseInstance)
    GET_GL_POINTER(DrawElementsInstancedBaseInstance)
    GET_GL_POINTER(DrawElementsInstancedBaseVertexBaseInstance)
    GET_GL_POINTER(DrawTransformFeedbackInstanced)
    GET_GL_POINTER(DrawTransformFeedbackStreamInstanced)
    #endif

    #ifdef GL_ARB_draw_instanced
    GET_GL_POINTER(DrawArraysInstancedARB)
    GET_GL_POINTER(DrawElementsInstancedARB)
    #endif

    #ifdef GL_EXT_draw_instanced
    GET_GL_POINTER(DrawArraysInstancedEXT)
    GET_GL_POINTER(DrawElementsInstancedEXT)
    #endif

    #ifdef GL_EXT_draw_range_elements
    GET_GL_POINTER(DrawRangeElementsEXT)
    #endif
    #ifdef GL_EXT_multi_draw_arrays
    GET_GL_POINTER(MultiDrawArraysEXT)
    GET_GL_POINTER(MultiDrawElementsEXT)
    #endif
    #ifdef GL_EXT_vertex_array
    GET_GL_POINTER(DrawArraysEXT)
    #endif
    
    procs_loaded = true;  
}

}
