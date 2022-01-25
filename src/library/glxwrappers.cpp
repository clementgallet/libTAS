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

#include "glxwrappers.h"
#include "openglwrappers.h"
#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_GL.h"
#include "ScreenCapture.h"
#include "frame.h"
#include "xlib/xwindows.h" // x11::gameXWindows
#include "GameHacks.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <string.h>

#define STORE_SYMBOL(str) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        return reinterpret_cast<void*>(orig::str); \
    }

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

DEFINE_ORIG_POINTER(glXGetProcAddress)
DEFINE_ORIG_POINTER(glXGetProcAddressARB)
DEFINE_ORIG_POINTER(glXGetProcAddressEXT)
DEFINE_ORIG_POINTER(glXMakeCurrent)
DEFINE_ORIG_POINTER(glXMakeContextCurrent)
DEFINE_ORIG_POINTER(glXSwapBuffers)
DEFINE_ORIG_POINTER(glXSwapIntervalEXT)
DEFINE_ORIG_POINTER(glXSwapIntervalSGI)
DEFINE_ORIG_POINTER(glXSwapIntervalMESA)
DEFINE_ORIG_POINTER(glXGetSwapIntervalMESA)
DEFINE_ORIG_POINTER(glXQueryExtensionsString)
DEFINE_ORIG_POINTER(glXQueryDrawable)
DEFINE_ORIG_POINTER(glXCreateContextAttribsARB)
DEFINE_ORIG_POINTER(glXDestroyContext)

DECLARE_ORIG_POINTER(glGetString)
DECLARE_ORIG_POINTER(glBlitFramebuffer)
DECLARE_ORIG_POINTER(glTexParameterf)
DECLARE_ORIG_POINTER(glTexParameteri)
DECLARE_ORIG_POINTER(glEnable)
DECLARE_ORIG_POINTER(glReadPixels)
DECLARE_ORIG_POINTER(glGenFramebuffers)
DECLARE_ORIG_POINTER(glBindFramebuffer)
DECLARE_ORIG_POINTER(glDeleteFramebuffers)
DECLARE_ORIG_POINTER(glGenRenderbuffers)
DECLARE_ORIG_POINTER(glBindRenderbuffer)
DECLARE_ORIG_POINTER(glDeleteRenderbuffers)
DECLARE_ORIG_POINTER(glRenderbufferStorage)
DECLARE_ORIG_POINTER(glFramebufferRenderbuffer)
DECLARE_ORIG_POINTER(glDisable)
DECLARE_ORIG_POINTER(glIsEnabled)
DECLARE_ORIG_POINTER(glGetIntegerv)
DECLARE_ORIG_POINTER(glGetError)
DECLARE_ORIG_POINTER(glGenTextures)
DECLARE_ORIG_POINTER(glDeleteTextures)
DECLARE_ORIG_POINTER(glBindTexture)
DECLARE_ORIG_POINTER(glBindSampler)
DECLARE_ORIG_POINTER(glTexImage2D)
DECLARE_ORIG_POINTER(glActiveTexture)
DECLARE_ORIG_POINTER(glFramebufferTexture2D)
DECLARE_ORIG_POINTER(glUseProgram)
DECLARE_ORIG_POINTER(glPixelStorei)

DECLARE_ORIG_POINTER(glGenBuffers)
DECLARE_ORIG_POINTER(glGenVertexArrays)
DECLARE_ORIG_POINTER(glBindVertexArray)
DECLARE_ORIG_POINTER(glBindBuffer)
DECLARE_ORIG_POINTER(glBufferData)
DECLARE_ORIG_POINTER(glVertexAttribPointer)
DECLARE_ORIG_POINTER(glEnableVertexAttribArray)
DECLARE_ORIG_POINTER(glCreateShader)
DECLARE_ORIG_POINTER(glShaderSource)
DECLARE_ORIG_POINTER(glCompileShader)
DECLARE_ORIG_POINTER(glGetShaderiv)
DECLARE_ORIG_POINTER(glGetShaderInfoLog)
DECLARE_ORIG_POINTER(glCreateProgram)
DECLARE_ORIG_POINTER(glAttachShader)
DECLARE_ORIG_POINTER(glLinkProgram)
DECLARE_ORIG_POINTER(glGetProgramiv)
DECLARE_ORIG_POINTER(glGetProgramInfoLog)
DECLARE_ORIG_POINTER(glDetachShader)
DECLARE_ORIG_POINTER(glDeleteShader)
DECLARE_ORIG_POINTER(glBlendFunc)
DECLARE_ORIG_POINTER(glDeleteBuffers)
DECLARE_ORIG_POINTER(glDeleteVertexArrays)
DECLARE_ORIG_POINTER(glDeleteProgram)

DECLARE_ORIG_POINTER(glClear)
DECLARE_ORIG_POINTER(glBegin)
DECLARE_ORIG_POINTER(glEnd)
DECLARE_ORIG_POINTER(glVertex2d)
DECLARE_ORIG_POINTER(glVertex2f)
DECLARE_ORIG_POINTER(glVertex2i)
DECLARE_ORIG_POINTER(glVertex2s)
DECLARE_ORIG_POINTER(glVertex3d)
DECLARE_ORIG_POINTER(glVertex3f)
DECLARE_ORIG_POINTER(glVertex3i)
DECLARE_ORIG_POINTER(glVertex3s)
DECLARE_ORIG_POINTER(glVertex4d)
DECLARE_ORIG_POINTER(glVertex4f)
DECLARE_ORIG_POINTER(glVertex4i)
DECLARE_ORIG_POINTER(glVertex4s)
DECLARE_ORIG_POINTER(glVertex2dv)
DECLARE_ORIG_POINTER(glVertex2fv)
DECLARE_ORIG_POINTER(glVertex2iv)
DECLARE_ORIG_POINTER(glVertex2sv)
DECLARE_ORIG_POINTER(glVertex3dv)
DECLARE_ORIG_POINTER(glVertex3fv)
DECLARE_ORIG_POINTER(glVertex3iv)
DECLARE_ORIG_POINTER(glVertex3sv)
DECLARE_ORIG_POINTER(glVertex4dv)
DECLARE_ORIG_POINTER(glVertex4fv)
DECLARE_ORIG_POINTER(glVertex4iv)
DECLARE_ORIG_POINTER(glVertex4sv)

DECLARE_ORIG_POINTER(glDrawArrays)
DECLARE_ORIG_POINTER(glDrawElements)
DECLARE_ORIG_POINTER(glMultiDrawArrays)
DECLARE_ORIG_POINTER(glMultiDrawElements)
DECLARE_ORIG_POINTER(glDrawRangeElements)
DECLARE_ORIG_POINTER(glDrawElementsBaseVertex)
DECLARE_ORIG_POINTER(glDrawRangeElementsBaseVertex)
DECLARE_ORIG_POINTER(glDrawElementsInstancedBaseVertex)
DECLARE_ORIG_POINTER(glMultiDrawElementsBaseVertex)
DECLARE_ORIG_POINTER(glDrawArraysInstancedBaseInstance)
DECLARE_ORIG_POINTER(glDrawElementsInstancedBaseInstance)
DECLARE_ORIG_POINTER(glDrawElementsInstancedBaseVertexBaseInstance)

DECLARE_ORIG_POINTER(glDrawTransformFeedback)
DECLARE_ORIG_POINTER(glDrawTransformFeedbackStream)
DECLARE_ORIG_POINTER(glDrawTransformFeedbackInstanced)
DECLARE_ORIG_POINTER(glDrawTransformFeedbackStreamInstanced)

DECLARE_ORIG_POINTER(glDrawArraysInstancedARB)
DECLARE_ORIG_POINTER(glDrawElementsInstancedARB)
DECLARE_ORIG_POINTER(glDrawArraysInstancedEXT)
DECLARE_ORIG_POINTER(glDrawElementsInstancedEXT)
DECLARE_ORIG_POINTER(glDrawRangeElementsEXT)
DECLARE_ORIG_POINTER(glMultiDrawArraysEXT)
DECLARE_ORIG_POINTER(glMultiDrawElementsEXT)
DECLARE_ORIG_POINTER(glDrawArraysEXT)

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

    debuglogstdio(LCF_OGL | LCF_INFO, "OpenGL vendor: %s", vendor);
    debuglogstdio(LCF_OGL | LCF_INFO, "OpenGL renderer: %s", renderer);

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

    /* Store function pointers that are used in other files */
    STORE_SYMBOL(glReadPixels)
    STORE_SYMBOL(glGenFramebuffers)
    STORE_SYMBOL(glBindFramebuffer)
    STORE_SYMBOL(glDeleteFramebuffers)
    STORE_SYMBOL(glGenRenderbuffers)
    STORE_SYMBOL(glBindRenderbuffer)
    STORE_SYMBOL(glDeleteRenderbuffers)
    STORE_SYMBOL(glRenderbufferStorage)
    STORE_SYMBOL(glFramebufferRenderbuffer)
    STORE_SYMBOL(glDisable)
    STORE_SYMBOL(glIsEnabled)
    STORE_SYMBOL(glGetIntegerv)
    STORE_SYMBOL(glGetError)

    STORE_SYMBOL(glGenTextures)
    STORE_SYMBOL(glDeleteTextures)
    STORE_SYMBOL(glBindTexture)
    STORE_SYMBOL(glBindSampler)
    STORE_SYMBOL(glTexImage2D)
    STORE_SYMBOL(glActiveTexture)
    STORE_SYMBOL(glFramebufferTexture2D)
    STORE_SYMBOL(glUseProgram)
    STORE_SYMBOL(glPixelStorei)

    STORE_SYMBOL(glGenBuffers)
    STORE_SYMBOL(glGenVertexArrays)
    STORE_SYMBOL(glBindVertexArray)
    STORE_SYMBOL(glBindBuffer)
    STORE_SYMBOL(glBufferData)
    STORE_SYMBOL(glVertexAttribPointer)
    STORE_SYMBOL(glEnableVertexAttribArray)
    STORE_SYMBOL(glCreateShader)
    STORE_SYMBOL(glShaderSource)
    STORE_SYMBOL(glCompileShader)
    STORE_SYMBOL(glGetShaderiv)
    STORE_SYMBOL(glGetShaderInfoLog)
    STORE_SYMBOL(glCreateProgram)
    STORE_SYMBOL(glAttachShader)
    STORE_SYMBOL(glLinkProgram)
    STORE_SYMBOL(glGetProgramiv)
    STORE_SYMBOL(glGetProgramInfoLog)
    STORE_SYMBOL(glDetachShader)
    STORE_SYMBOL(glDeleteShader)
    STORE_SYMBOL(glBlendFunc)
    STORE_SYMBOL(glDeleteBuffers)
    STORE_SYMBOL(glDeleteVertexArrays)
    STORE_SYMBOL(glDeleteProgram)

    /* Store function pointers and return our function */
    STORE_RETURN_SYMBOL(glXMakeCurrent)
    STORE_RETURN_SYMBOL(glXMakeContextCurrent)
    STORE_RETURN_SYMBOL(glXSwapBuffers)
    STORE_RETURN_SYMBOL(glXQueryDrawable)
    STORE_RETURN_SYMBOL(glXSwapIntervalEXT)
    STORE_RETURN_SYMBOL(glXSwapIntervalSGI)
    STORE_RETURN_SYMBOL(glXSwapIntervalMESA)
    STORE_RETURN_SYMBOL(glXGetSwapIntervalMESA)
    STORE_RETURN_SYMBOL(glXSwapIntervalSGI)
    STORE_RETURN_SYMBOL(glXQueryExtensionsString)
    STORE_RETURN_SYMBOL(glXCreateContextAttribsARB)
    STORE_RETURN_SYMBOL(glXDestroyContext)

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
    debuglogstdio(LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(glXGetProcAddress, "GL");

    if (!orig::glXGetProcAddress) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddress(procName))));
}

__GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName)
{
    debuglogstdio(LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(glXGetProcAddressARB, "GL");

    if (!orig::glXGetProcAddressARB) return nullptr;

    return reinterpret_cast<__GLXextFuncPtr>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddressARB(procName))));
}

void* glXGetProcAddressEXT (const GLubyte *procName)
{
    debuglogstdio(LCF_OGL, "%s call with symbol %s", __func__, procName);
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

    if (drawable && (!x11::gameXWindows.empty())) {

        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;

        /* If we are using SDL, we let the higher function initialize stuff */
        if (!(game_info.video & (GameInfo::SDL1 | GameInfo::SDL2 | GameInfo::SDL2_RENDERER | GameInfo::VDPAU))) {
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

Bool glXMakeContextCurrent( Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx )
{
    LINK_NAMESPACE(glXMakeContextCurrent, "GL");

    Bool ret = orig::glXMakeContextCurrent(dpy, draw, read, ctx);
    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

    if (draw && (!x11::gameXWindows.empty())) {

        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;

        /* If we are using SDL, we let the higher function initialize stuff */
        if (!(game_info.video & (GameInfo::SDL1 | GameInfo::SDL2 | GameInfo::SDL2_RENDERER | GameInfo::VDPAU))) {
            /* Now that the context is created, we can init the screen capture */
            ScreenCapture::init();
        }

        checkMesa();
    }

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
    frameBoundary([&] () {orig::glXSwapBuffers(dpy, drawable);}, renderHUD);
#else
    frameBoundary([&] () {orig::glXSwapBuffers(dpy, drawable);});
#endif
}

static int swapInterval = 0;

void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval)
{
    debuglogstdio(LCF_OGL, "%s call with interval %d", __func__, interval);
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
    debuglogstdio(LCF_OGL, "%s call with interval %d", __func__, interval);
    LINK_NAMESPACE(glXSwapIntervalSGI, "GL");

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return orig::glXSwapIntervalSGI(interval);
    }
    else {
        int ret = orig::glXSwapIntervalSGI(1);
        debuglogstdio(LCF_OGL, "   ret %d", ret);
        return ret;
    }
}

int glXSwapIntervalMESA (unsigned int interval)
{
    debuglogstdio(LCF_OGL, "%s call with interval %d", __func__, interval);
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
    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);
    return swapInterval;
}

const char* glXQueryExtensionsString(Display* dpy, int screen)
{
    DEBUGLOGCALL(LCF_OGL);

    /* Unity has different behaviors depending on the GLX extensions present,
     * at least for GLX_SGI_swap_control. Returning an empty string works fine. */
    if (GameHacks::isUnity())
        return "GLX_ARB_create_context GLX_ARB_create_context_profile \
GLX_ARB_create_context_robustness GLX_ARB_fbconfig_float \
GLX_ARB_framebuffer_sRGB GLX_ARB_get_proc_address GLX_ARB_multisample \
GLX_EXT_create_context_es2_profile GLX_EXT_create_context_es_profile \
GLX_EXT_fbconfig_packed_float GLX_EXT_framebuffer_sRGB \
GLX_EXT_import_context GLX_EXT_texture_from_pixmap GLX_EXT_visual_info \
GLX_EXT_visual_rating GLX_MESA_copy_sub_buffer GLX_MESA_query_renderer \
GLX_OML_swap_method GLX_SGIS_multisample GLX_SGIX_fbconfig \
GLX_SGIX_pbuffer GLX_SGIX_visual_select_group GLX_SGI_make_current_read";

    LINK_NAMESPACE(glXQueryExtensionsString, "GL");
    return orig::glXQueryExtensionsString(dpy, screen);
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

void glXDestroyContext(Display * dpy,  GLXContext ctx)
{
    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);
    LINK_NAMESPACE(glXDestroyContext, "GL");
    ScreenCapture::fini();
    return orig::glXDestroyContext(dpy, ctx);
}

}
