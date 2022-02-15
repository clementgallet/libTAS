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

#include "eglwrappers.h"
#include "openglwrappers.h"
#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_GL.h"
#include "ScreenCapture.h"
#include "frame.h"
#include "xlib/xwindows.h" // x11::gameXWindows

#include <string.h>

#define STORE_SYMBOL(str) \
    if (!strcmp(symbol, #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_OGL,"  store real function in %p", real_pointer); \
        return reinterpret_cast<void*>(orig::str); \
    }

#define STORE_RETURN_SYMBOL(str) \
    if (!strcmp(symbol, #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_OGL,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(libtas::str), real_pointer); \
        return reinterpret_cast<void*>(libtas::str); \
    }

#define STORE_RETURN_SYMBOL_CUSTOM(str) \
    if (!strcmp(symbol, #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_OGL,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(my##str), real_pointer); \
        return reinterpret_cast<void*>(my##str); \
    }

namespace libtas {

DEFINE_ORIG_POINTER(eglGetProcAddress)
DEFINE_ORIG_POINTER(eglMakeCurrent)
DEFINE_ORIG_POINTER(eglSwapBuffers)
DEFINE_ORIG_POINTER(eglSwapInterval)
DEFINE_ORIG_POINTER(eglBindAPI)
DEFINE_ORIG_POINTER(eglCreateContext)

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

/* If the game uses the eglGetProcAddress functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* store_orig_and_return_my_symbol(const char* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    STORE_RETURN_SYMBOL(eglMakeCurrent)
    STORE_RETURN_SYMBOL(eglSwapBuffers)
    STORE_RETURN_SYMBOL(eglSwapInterval)
    STORE_RETURN_SYMBOL(eglBindAPI)
    STORE_RETURN_SYMBOL(eglCreateContext)

    /* Store function pointers that are used in other files */
    STORE_SYMBOL(glGetString)
    STORE_SYMBOL(glBlitFramebuffer);
    STORE_SYMBOL(glTexParameterf);
    STORE_SYMBOL(glTexParameteri);
    STORE_SYMBOL(glEnable);
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
    
    return real_pointer;
}

void(*eglGetProcAddress (const char *procName))()
{
    debuglogstdio(LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(eglGetProcAddress, "EGL");

    if (!orig::eglGetProcAddress) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::eglGetProcAddress(procName))));
}

EGLBoolean eglMakeCurrent( EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx )
{
    LINK_NAMESPACE(eglMakeCurrent, "EGL");

    EGLBoolean ret = orig::eglMakeCurrent(dpy, draw, read, ctx);
    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

    if (draw && (!x11::gameXWindows.empty())) {

        game_info.video |= GameInfo::EGL | GameInfo::OPENGL;
        game_info.tosend = true;

        /* If we are using SDL, we let the higher function initialize stuff */
        if (!(game_info.video & (GameInfo::SDL1 | GameInfo::SDL2 | GameInfo::SDL2_RENDERER))) {
            /* Now that the context is created, we can init the screen capture */
            ScreenCapture::init();
        }
    }

    return ret;
}

static EGLenum bindAPI = EGL_OPENGL_ES_API;

EGLBoolean eglBindAPI(EGLenum api)
{
    debuglogstdio(LCF_OGL, "%s call with API %d", __func__, api);
    LINK_NAMESPACE(eglBindAPI, "EGL");

    bindAPI = api;

    return orig::eglBindAPI(api);
}

EGLBoolean eglSwapBuffers( EGLDisplay dpy, EGLSurface surface )
{
    LINK_NAMESPACE(eglSwapBuffers, "EGL");

    if (GlobalState::isNative())
        return orig::eglSwapBuffers(dpy, surface);

    DEBUGLOGCALL(LCF_WINDOW | LCF_OGL);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    // static RenderHUD_GL renderHUD;
    static RenderHUD_GL renderHUD;
    renderHUD.setGLES(bindAPI == EGL_OPENGL_ES_API);
    frameBoundary([&] () {orig::eglSwapBuffers(dpy, surface);}, renderHUD);
#else
    frameBoundary([&] () {orig::eglSwapBuffers(dpy, surface);});
#endif

    return EGL_TRUE;
}

static int swapInterval = 0;

EGLBoolean eglSwapInterval (EGLDisplay dpy, EGLint interval)
{
    debuglogstdio(LCF_OGL, "%s call with interval %d", __func__, interval);
    LINK_NAMESPACE(eglSwapInterval, "EGL");

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return orig::eglSwapInterval(dpy, interval);
    }
    return orig::eglSwapInterval(dpy, interval);
}

EGLContext eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    DEBUGLOGCALL(LCF_OGL);
    LINK_NAMESPACE(eglCreateContext, "EGL");
    return orig::eglCreateContext (dpy, config, share_context, attrib_list);
}

}
