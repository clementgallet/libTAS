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
#include "renderhud/RenderHUD_GL.h"
#include "ScreenCapture.h"
#include "frame.h"

#define STORE_RETURN_SYMBOL(str) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_OGL,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(libtas::str), real_pointer); \
        return reinterpret_cast<void*>(libtas::str); \
    }

#define STORE_RETURN_SYMBOL_CUSTOM(str) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_OGL,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(my##str), real_pointer); \
        return reinterpret_cast<void*>(my##str); \
    }

namespace libtas {

#ifdef LIBTAS_ENABLE_HUD
static RenderHUD_GL renderHUD_GL;
#endif

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
DEFINE_ORIG_POINTER(glXCreateContextAttribsARB);

DEFINE_ORIG_POINTER(glGetString);

DEFINE_ORIG_POINTER(glBlitFramebuffer);

DEFINE_ORIG_POINTER(glTexParameterf);
DEFINE_ORIG_POINTER(glTexParameteri);
// DEFINE_ORIG_POINTER(glTexParameterfv);
// DEFINE_ORIG_POINTER(glTexParameteriv);
// DEFINE_ORIG_POINTER(glTexParameterIiv);
// DEFINE_ORIG_POINTER(glTexParameterIuiv);

DEFINE_ORIG_POINTER(glEnable);
// DEFINE_ORIG_POINTER(glDisable);


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
GLFUNCSKIPDRAW(glDrawArraysInstancedBaseInstance, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance), (mode, first, count, instancecount, baseinstance))
GLFUNCSKIPDRAW(glDrawElementsInstancedBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance), (mode, count, type, indices, instancecount, baseinstance))
GLFUNCSKIPDRAW(glDrawElementsInstancedBaseVertexBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance), (mode, count, type, indices, instancecount, basevertex, baseinstance))

GLFUNCSKIPDRAW(glDrawTransformFeedback, (GLenum mode, GLuint id), (mode, id))
GLFUNCSKIPDRAW(glDrawTransformFeedbackStream, (GLenum mode, GLuint id, GLuint stream), (mode, id, stream))
GLFUNCSKIPDRAW(glDrawTransformFeedbackInstanced, (GLenum mode, GLuint id, GLsizei instancecount), (mode, id, instancecount))
GLFUNCSKIPDRAW(glDrawTransformFeedbackStreamInstanced, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount), (mode, id, stream, instancecount))

GLFUNCSKIPDRAW(glDrawArraysInstancedARB, (GLenum mode, GLint first, GLsizei count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(glDrawElementsInstancedARB, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
GLFUNCSKIPDRAW(glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount), (mode, start, count, primcount))
GLFUNCSKIPDRAW(glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount), (mode, count, type, indices, primcount))
GLFUNCSKIPDRAW(glDrawRangeElementsEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices), (mode, start, end, count, type, indices))
GLFUNCSKIPDRAW(glMultiDrawArraysEXT, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount), (mode, first, count, primcount))
GLFUNCSKIPDRAW(glMultiDrawElementsEXT, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount), (mode, count, type, indices, primcount))
GLFUNCSKIPDRAW(glDrawArraysEXT, (GLenum mode, GLint first, GLsizei count), (mode, first, count))

void checkMesa()
{
    /* Check only once */
    static bool checked = false;
    if (checked) return;
    checked = true;

    /* Get OpenGL vendor and renderer */
    LINK_NAMESPACE(glGetString, "GL");
    const char* vendor = reinterpret_cast<const char*>(orig::glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(orig::glGetString(GL_RENDERER));

    if (!vendor) {
        checked = false;
        return;
    }

    debuglog(LCF_OGL | LCF_INFO, "OpenGL vendor: ", vendor);
    debuglog(LCF_OGL | LCF_INFO, "OpenGL renderer: ", renderer);

    /* Check that we are using llvm driver */
    if (shared_config.opengl_soft && (strstr(renderer, "llvmpipe") == nullptr)) {
        /* Alert the user that software rendering is not enabled */
        std::string alertMsg = "Software rendering was enabled, but the OpenGL renderer currently used (";
        alertMsg += renderer;
        alertMsg += ") does not match. Check if you have a Mesa-compatible GPU driver installed. At the moment, it is likely that savestates will crash";
        sendAlertMsg(alertMsg);
    }
}

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
    STORE_RETURN_SYMBOL(glXCreateContextAttribsARB)

    /* Some games like Super Meat Boy defines the glDrawArrays function in the
     * executable, so even if we preload our glDrawArrays function, it will still
     * refer to the one in the executable. So we rename our function to some
     * other name. This is not a problem because games should not call these
     * opengl functions directly but by using a glGetProcAddress function.
     */
    STORE_RETURN_SYMBOL_CUSTOM(glClear)

    STORE_RETURN_SYMBOL_CUSTOM(glBegin);
    STORE_RETURN_SYMBOL_CUSTOM(glEnd);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex2d);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2f);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2i);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2s);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex3d);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3f);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3i);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3s);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex4d);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4f);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4i);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4s);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex2dv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2fv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2iv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex2sv);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex3dv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3fv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3iv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex3sv);

    STORE_RETURN_SYMBOL_CUSTOM(glVertex4dv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4fv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4iv);
    STORE_RETURN_SYMBOL_CUSTOM(glVertex4sv);

    STORE_RETURN_SYMBOL_CUSTOM(glDrawArrays)
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElements)
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawArrays)
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElements)

    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElements);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElementsBaseVertex);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedBaseInstance);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseInstance);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertexBaseInstance);

    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedback);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStream);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackInstanced);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStreamInstanced);

    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedARB);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedARB);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawRangeElementsEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawArraysEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glMultiDrawElementsEXT);
    STORE_RETURN_SYMBOL_CUSTOM(glDrawArraysEXT);

    STORE_RETURN_SYMBOL_CUSTOM(glBlitFramebuffer);

    STORE_RETURN_SYMBOL_CUSTOM(glTexParameterf);
    STORE_RETURN_SYMBOL_CUSTOM(glTexParameteri);
    // STORE_RETURN_SYMBOL_CUSTOM(glTexParameterfv);
    // STORE_RETURN_SYMBOL_CUSTOM(glTexParameteriv);
    // STORE_RETURN_SYMBOL_CUSTOM(glTexParameterIiv);
    // STORE_RETURN_SYMBOL_CUSTOM(glTexParameterIuiv);

    STORE_RETURN_SYMBOL_CUSTOM(glEnable);
    // STORE_RETURN_SYMBOL_CUSTOM(glDisable);

    return real_pointer;
}

void(*glXGetProcAddress (const GLubyte *procName))()
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddress, "GL");

    if (!orig::glXGetProcAddress) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddress(procName))));
}

__GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName)
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressARB, "GL");

    if (!orig::glXGetProcAddressARB) return nullptr;

    return reinterpret_cast<__GLXextFuncPtr>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddressARB(procName))));
}

void* glXGetProcAddressEXT (const GLubyte *procName)
{
    debuglog(LCF_OGL, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressEXT, "GL");

    if (!orig::glXGetProcAddressEXT) return nullptr;

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddressEXT(procName));
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
    LINK_NAMESPACE(glXMakeCurrent, "GL");

    Bool ret = orig::glXMakeCurrent(dpy, drawable, ctx);
    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

    if (drawable && (!gameXWindows.empty())) {

        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;

        /* If we are using SDL, we let the higher function initialize stuff */
        if (!(game_info.video & (GameInfo::SDL1 | GameInfo::SDL2 | GameInfo::SDL2_RENDERER))) {
            /* Now that the context is created, we can init the screen capture */
            ScreenCapture::init();
        }

        checkMesa();
    }

    /* Disable VSync */
    //LINK_NAMESPACE(glXGetProcAddressARB, "GL");
    //orig::glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)orig::glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    //orig::glXSwapIntervalEXT(dpy, drawable, 0);

    return ret;
}

void glXSwapBuffers( Display *dpy, XID drawable )
{
    LINK_NAMESPACE(glXSwapBuffers, "GL");

    if (GlobalState::isNative())
        return orig::glXSwapBuffers(dpy, drawable);

    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

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
    LINK_NAMESPACE(glXSwapIntervalEXT, "GL");

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        orig::glXSwapIntervalEXT(dpy, drawable, interval);
    }
    else {
        orig::glXSwapIntervalEXT(dpy, drawable, 0);
    }

}

int glXSwapIntervalSGI (int interval)
{
    debuglog(LCF_OGL, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalSGI, "GL");

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return orig::glXSwapIntervalSGI(interval);
    }
    else {
        return orig::glXSwapIntervalSGI(0);
    }
}

int glXSwapIntervalMESA (unsigned int interval)
{
    debuglog(LCF_OGL, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalMESA, "GL");

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return orig::glXSwapIntervalMESA(interval);
    }
    else {
        return orig::glXSwapIntervalMESA(0);
    }
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

    LINK_NAMESPACE(glXQueryDrawable, "GL");
    return orig::glXQueryDrawable(dpy, draw, attribute, value);
}

GLXContext glXCreateContextAttribsARB (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list)
{
    DEBUGLOGCALL(LCF_OGL);
    LINK_NAMESPACE(glXCreateContextAttribsARB, "GL");
    int i = 0;
    while (attrib_list[i] != 0) {
        if (attrib_list[i] == GLX_CONTEXT_MAJOR_VERSION_ARB) {
            game_info.opengl_major = attrib_list[i+1];
            game_info.tosend = true;
        }
        if (attrib_list[i] == GLX_CONTEXT_MINOR_VERSION_ARB) {
            game_info.opengl_minor = attrib_list[i+1];
            game_info.tosend = true;
        }
        if (attrib_list[i] == GLX_CONTEXT_PROFILE_MASK_ARB) {
            switch(attrib_list[i+1]) {
            case GLX_CONTEXT_CORE_PROFILE_BIT_ARB:
                game_info.opengl_profile = GameInfo::CORE;
                break;
            case GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB:
                game_info.opengl_profile = GameInfo::COMPATIBILITY;
                break;
            default:
                break;
            }
            game_info.tosend = true;
        }

        i += 2;
    }
    return orig::glXCreateContextAttribsARB (dpy, config, share_context, direct, attrib_list);
}

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
