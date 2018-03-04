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

#include "openglwrappers.h"
#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_GL.h"
#include "ScreenCapture.h"
#include "frame.h"

#define STORE_RETURN_SYMBOL(str) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        return reinterpret_cast<void*>(str); \
    }

#define STORE_RETURN_SYMBOL_CUSTOM(str,myfunc) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        return reinterpret_cast<void*>(myfunc); \
    }

namespace libtas {

DEFINE_ORIG_POINTER(glXGetProcAddress);
DEFINE_ORIG_POINTER(glXGetProcAddressARB);
DEFINE_ORIG_POINTER(glXGetProcAddressEXT);
DEFINE_ORIG_POINTER(glXMakeCurrent);
DEFINE_ORIG_POINTER(glXSwapBuffers);
DEFINE_ORIG_POINTER(glXSwapIntervalEXT);
DEFINE_ORIG_POINTER(glXSwapIntervalSGI);
DEFINE_ORIG_POINTER(glXSwapIntervalMESA);
DEFINE_ORIG_POINTER(glXGetSwapIntervalMESA);
DEFINE_ORIG_POINTER(glXQueryDrawable);
DEFINE_ORIG_POINTER(glDrawArrays);
DEFINE_ORIG_POINTER(glDrawElements);
DEFINE_ORIG_POINTER(glMultiDrawArrays);
DEFINE_ORIG_POINTER(glMultiDrawElements);

DEFINE_ORIG_POINTER(glDrawRangeElements);
DEFINE_ORIG_POINTER(glDrawElementsBaseVertex);
DEFINE_ORIG_POINTER(glDrawRangeElementsBaseVertex);
DEFINE_ORIG_POINTER(glDrawElementsInstancedBaseVertex);
DEFINE_ORIG_POINTER(glMultiDrawElementsBaseVertex);
DEFINE_ORIG_POINTER(glDrawArraysInstancedBaseInstance);
DEFINE_ORIG_POINTER(glDrawElementsInstancedBaseInstance);
DEFINE_ORIG_POINTER(glDrawElementsInstancedBaseVertexBaseInstance);

DEFINE_ORIG_POINTER(glDrawTransformFeedback);
DEFINE_ORIG_POINTER(glDrawTransformFeedbackStream);
DEFINE_ORIG_POINTER(glDrawTransformFeedbackInstanced);
DEFINE_ORIG_POINTER(glDrawTransformFeedbackStreamInstanced);

DEFINE_ORIG_POINTER(glDrawArraysInstancedARB);
DEFINE_ORIG_POINTER(glDrawElementsInstancedARB);
DEFINE_ORIG_POINTER(glDrawArraysInstancedEXT);
DEFINE_ORIG_POINTER(glDrawElementsInstancedEXT);
DEFINE_ORIG_POINTER(glDrawRangeElementsEXT);
DEFINE_ORIG_POINTER(glMultiDrawArraysEXT);
DEFINE_ORIG_POINTER(glMultiDrawElementsEXT);
DEFINE_ORIG_POINTER(glDrawArraysEXT);


/* If the game uses the glXGetProcAddressXXX functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* store_orig_and_return_my_symbol(const GLubyte* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    // if (strcmp(reinterpret_cast<const char*>(symbol), "glDrawArrays") == 0) {
    //     debuglog(LCF_OGL, __func__, "   hooking  glDrawArrays");
    //     debuglog(LCF_OGL, __func__, "   real pointer is ", real_pointer);
    //     debuglog(LCF_OGL, __func__, "   my pointer is ", reinterpret_cast<void*>(myglDrawArrays));
    //     orig::glDrawArrays = reinterpret_cast<decltype(orig::glDrawArrays)>(real_pointer);
    //     return reinterpret_cast<void*>(myglDrawArrays);
    // }

    STORE_RETURN_SYMBOL(glXMakeCurrent)
    STORE_RETURN_SYMBOL(glXSwapBuffers)
    STORE_RETURN_SYMBOL(glXQueryDrawable)
    STORE_RETURN_SYMBOL(glXSwapIntervalEXT)
    STORE_RETURN_SYMBOL(glXSwapIntervalSGI)
    STORE_RETURN_SYMBOL(glXSwapIntervalMESA)
    STORE_RETURN_SYMBOL(glXGetSwapIntervalMESA)
    STORE_RETURN_SYMBOL(glXSwapIntervalSGI)

    /* Some games like Super Meat Boy defines the glDrawArrays function in the
     * executable, so even if we preload our glDrawArrays function, it will still
     * refer to the one in the executable. So we rename our function to some
     * other name. This is not a problem because games should not call these
     * opengl functions directly but by using a glGetProcAddress function.
     */
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArrays, myglDrawArrays)
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElements, myglDrawElements)
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawArrays, myglMultiDrawArrays)
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElements, myglMultiDrawElements)

    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElements, myglDrawRangeElements);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsBaseVertex, myglDrawElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElementsBaseVertex, myglDrawRangeElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertex, myglDrawElementsInstancedBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElementsBaseVertex, myglMultiDrawElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedBaseInstance, myglDrawArraysInstancedBaseInstance);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseInstance, myglDrawElementsInstancedBaseInstance);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertexBaseInstance, myglDrawElementsInstancedBaseVertexBaseInstance);

    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedback, myglDrawTransformFeedback);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStream, myglDrawTransformFeedbackStream);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackInstanced, myglDrawTransformFeedbackInstanced);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStreamInstanced, myglDrawTransformFeedbackStreamInstanced);

    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedARB, myglDrawArraysInstancedARB);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedARB, myglDrawElementsInstancedARB);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedEXT, myglDrawArraysInstancedEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedEXT, myglDrawElementsInstancedEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElementsEXT, myglDrawRangeElementsEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawArraysEXT, myglMultiDrawArraysEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElementsEXT, myglMultiDrawElementsEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysEXT, myglDrawArraysEXT);

    return real_pointer;
}

void(*glXGetProcAddress (const GLubyte *procName))()
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddress, "libGL");

    if (!orig::glXGetProcAddress) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddress(procName))));
}

__GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName)
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressARB, "libGL");

    if (!orig::glXGetProcAddressARB) return nullptr;

    return reinterpret_cast<__GLXextFuncPtr>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddressARB(procName))));
}

void* glXGetProcAddressEXT (const GLubyte *procName)
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressEXT, "libGL");

    if (!orig::glXGetProcAddressEXT) return nullptr;

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddressEXT(procName));
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);
    LINK_NAMESPACE(glXMakeCurrent, "libGL");

    Bool ret = orig::glXMakeCurrent(dpy, drawable, ctx);

    if (drawable) {
        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;

        /* Now that the context is created, we can init the screen capture */
        ScreenCapture::init(nullptr);
    }

    /* Disable VSync */
    //LINK_NAMESPACE(glXGetProcAddressARB, "libGL");
    //orig::glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)orig::glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    //orig::glXSwapIntervalEXT(dpy, drawable, 0);

    return ret;
}

void glXSwapBuffers( Display *dpy, XID drawable )
{
    LINK_NAMESPACE(glXSwapBuffers, "libGL");

    if (GlobalState::isNative())
        return orig::glXSwapBuffers(dpy, drawable);

    DEBUGLOGCALL(LCF_FRAME | LCF_WINDOW | LCF_OGL);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_GL renderHUD;
    frameBoundary(true, [&] () {orig::glXSwapBuffers(dpy, drawable);}, renderHUD);
#else
    frameBoundary(true, [&] () {orig::glXSwapBuffers(dpy, drawable);});
#endif
}

static int swapInterval = 0;

void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval)
{
    debuglog(LCF_OGL, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalEXT, "libGL");

    /* Store the interval but don't set it */
    swapInterval = interval;
    orig::glXSwapIntervalEXT(dpy, drawable, 0);
}

int glXSwapIntervalSGI (int interval)
{
    debuglog(LCF_OGL, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalSGI, "libGL");

    /* Store the interval but don't set it */
    swapInterval = interval;
    return orig::glXSwapIntervalSGI(0);
}

int glXSwapIntervalMESA (unsigned int interval)
{
    debuglog(LCF_OGL, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalMESA, "libGL");

    /* Store the interval but don't set it */
    swapInterval = interval;
    return orig::glXSwapIntervalMESA(0);
}

int glXGetSwapIntervalMESA(void)
{
    DEBUGLOGCALL(LCF_WINDOW);
    return swapInterval;
}

void glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value)
{
    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

    if (attribute == GLX_SWAP_INTERVAL_EXT) {
        *value = swapInterval;
        return;
    }
    if (attribute == GLX_MAX_SWAP_INTERVAL_EXT) {
        *value = 1;
        return;
    }

    LINK_NAMESPACE(glXQueryDrawable, "libGL");
    return orig::glXQueryDrawable(dpy, draw, attribute, value);
}

void glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
    DEBUGLOGCALL(LCF_OGL);
    LINK_NAMESPACE(glDrawArrays, "libGL");
    if (!skipping_draw)
        return orig::glDrawArrays(mode, first, count);
}

void glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
    DEBUGLOGCALL(LCF_OGL);
    LINK_NAMESPACE(glDrawElements, "libGL");
    if (!skipping_draw)
        return orig::glDrawElements(mode, count, type, indices);
}

void myglDrawArrays( GLenum mode, GLint first, GLsizei count )
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawArrays(mode, first, count);
}

void myglDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElements(mode, count, type, indices);
}

void myglMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glMultiDrawArrays(mode, first, count, drawcount);
}

void myglMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glMultiDrawElements(mode, count, type, indices, drawcount);
}

void myglDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawRangeElements(mode, start, end, count, type, indices);
}

void myglDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void myglDrawRangeElementsBaseVertex (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void myglDrawElementsInstancedBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

void myglMultiDrawElementsBaseVertex (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

void myglDrawArraysInstancedBaseInstance (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
}

void myglDrawElementsInstancedBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
}

void myglDrawElementsInstancedBaseVertexBaseInstance (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance);
}

void myglDrawTransformFeedback (GLenum mode, GLuint id)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawTransformFeedback(mode, id);
}

void myglDrawTransformFeedbackStream (GLenum mode, GLuint id, GLuint stream)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawTransformFeedbackStream(mode, id, stream);
}

void myglDrawTransformFeedbackInstanced (GLenum mode, GLuint id, GLsizei instancecount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawTransformFeedbackInstanced(mode, id, instancecount);
}

void myglDrawTransformFeedbackStreamInstanced (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount);
}

void myglDrawArraysInstancedARB (GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawArraysInstancedARB(mode, first, count, primcount);
}

void myglDrawElementsInstancedARB (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsInstancedARB(mode, count, type, indices, primcount);
}

void myglDrawArraysInstancedEXT (GLenum mode, GLint start, GLsizei count, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawArraysInstancedEXT(mode, start, count, primcount);
}

void myglDrawElementsInstancedEXT (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawElementsInstancedEXT(mode, count, type, indices, primcount);
}

void myglDrawRangeElementsEXT (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawRangeElementsEXT(mode, start, end, count, type, indices);
}

void myglMultiDrawArraysEXT (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glMultiDrawArraysEXT(mode, first, count, primcount);
}

void myglMultiDrawElementsEXT (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glMultiDrawElementsEXT(mode, count, type, indices, primcount);
}

void myglDrawArraysEXT (GLenum mode, GLint first, GLsizei count)
{
    DEBUGLOGCALL(LCF_OGL);
    if (!skipping_draw)
        return orig::glDrawArraysEXT(mode, first, count);
}

}
