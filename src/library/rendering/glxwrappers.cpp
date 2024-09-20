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

#include "glxwrappers.h"
#include "openglloader.h"
#include "openglwrappers.h"

#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_GL.h"
#include "screencapture/ScreenCapture.h"
#include "frame.h"
#include "xlib/XlibGameWindow.h"
#include "UnityHacks.h"
#include "global.h"
#include "GlobalState.h"

#define GLX_GLXEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <string.h>

#define RETURN_SYMBOL_DETAILED(FUNC_STR, NEW_FUNC) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), FUNC_STR)) { \
        if (NEW_FUNC) { \
            LOG(LL_DEBUG, LCF_OGL,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(NEW_FUNC), real_pointer); \
            return reinterpret_cast<void*>(NEW_FUNC); \
        } \
        return real_pointer; \
    }

#define RETURN_SYMBOL(FUNC) RETURN_SYMBOL_DETAILED(#FUNC, libtas::FUNC)
#define RETURN_SYMBOL_CUSTOM(FUNC) RETURN_SYMBOL_DETAILED(#FUNC, my##FUNC)
#define REPLACE_SYMBOL(FUNC_STR, ORIG_FUNC, NEW_FUNC) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), FUNC_STR)) { \
        ORIG_FUNC = reinterpret_cast<decltype(ORIG_FUNC)>(real_pointer); \
    } \
    RETURN_SYMBOL_DETAILED(FUNC_STR, NEW_FUNC)

namespace libtas {

DEFINE_ORIG_POINTER(glXGetProcAddress)
DEFINE_ORIG_POINTER(glXGetProcAddressARB)
DEFINE_ORIG_POINTER(glXGetProcAddressEXT)
void checkMesa()
{
    /* Check only once */
    static bool checked = false;
    if (checked) return;
    checked = true;

    /* Get OpenGL vendor and renderer */
    LINK_GL_POINTER(GetString);
    const char* vendor = reinterpret_cast<const char*>(glProcs.GetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glProcs.GetString(GL_RENDERER));

    if (!vendor) {
        checked = false;
        return;
    }

    LOG(LL_INFO, LCF_OGL, "OpenGL vendor: %s", vendor);
    LOG(LL_INFO, LCF_OGL, "OpenGL renderer: %s", renderer);

    /* Check that we are using llvm driver */
    if (Global::shared_config.opengl_soft && (strstr(renderer, "llvmpipe") == nullptr)) {
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


    /* Old extension that became core. I load into the same function pointers
     * for easier usage */
    REPLACE_SYMBOL("glBindFramebufferEXT", glProcs.BindFramebuffer, NULL)
    REPLACE_SYMBOL("glGenFramebuffersEXT", glProcs.GenFramebuffers, NULL)
    REPLACE_SYMBOL("glFramebufferTexture2DEXT", glProcs.FramebufferTexture2D, NULL)
    REPLACE_SYMBOL("glGenRenderbuffersEXT", glProcs.GenRenderbuffers, NULL)
    REPLACE_SYMBOL("glBindRenderbufferEXT", glProcs.BindRenderbuffer, NULL)
    REPLACE_SYMBOL("glRenderbufferStorageEXT", glProcs.RenderbufferStorage, NULL)
    REPLACE_SYMBOL("glFramebufferRenderbufferEXT", glProcs.FramebufferRenderbuffer, NULL)
    REPLACE_SYMBOL("glDeleteRenderbuffersEXT", glProcs.DeleteRenderbuffers, NULL)
    REPLACE_SYMBOL("glDeleteFramebuffersEXT", glProcs.DeleteFramebuffers, NULL)
    REPLACE_SYMBOL("glBlitFramebufferEXT", glProcs.BlitFramebuffer, myglBlitFramebuffer)

    /* Store function pointers and return our function */
    RETURN_SYMBOL(glXMakeCurrent)
    RETURN_SYMBOL(glXMakeContextCurrent)
    RETURN_SYMBOL(glXSwapBuffers)
    RETURN_SYMBOL(glXQueryDrawable)
    RETURN_SYMBOL(glXSwapIntervalEXT)
    RETURN_SYMBOL(glXSwapIntervalSGI)
    RETURN_SYMBOL(glXSwapIntervalMESA)
    RETURN_SYMBOL(glXGetSwapIntervalMESA)
    RETURN_SYMBOL(glXSwapIntervalSGI)
    RETURN_SYMBOL(glXQueryExtensionsString)
    RETURN_SYMBOL(glXCreateContextAttribsARB)
    RETURN_SYMBOL(glXDestroyContext)

    /* Some games like Super Meat Boy defines the glDrawArrays function in the
     * executable, so even if we preload our glDrawArrays function, it will still
     * refer to the one in the executable. So we rename our function to some
     * other name. This is not a problem because games should not call these
     * opengl functions directly but by using a glGetProcAddress function.
     */
    RETURN_SYMBOL_CUSTOM(glClear)

    RETURN_SYMBOL_CUSTOM(glBegin);
    RETURN_SYMBOL_CUSTOM(glEnd);

    RETURN_SYMBOL_CUSTOM(glVertex2d);
    RETURN_SYMBOL_CUSTOM(glVertex2f);
    RETURN_SYMBOL_CUSTOM(glVertex2i);
    RETURN_SYMBOL_CUSTOM(glVertex2s);

    RETURN_SYMBOL_CUSTOM(glVertex3d);
    RETURN_SYMBOL_CUSTOM(glVertex3f);
    RETURN_SYMBOL_CUSTOM(glVertex3i);
    RETURN_SYMBOL_CUSTOM(glVertex3s);

    RETURN_SYMBOL_CUSTOM(glVertex4d);
    RETURN_SYMBOL_CUSTOM(glVertex4f);
    RETURN_SYMBOL_CUSTOM(glVertex4i);
    RETURN_SYMBOL_CUSTOM(glVertex4s);

    RETURN_SYMBOL_CUSTOM(glVertex2dv);
    RETURN_SYMBOL_CUSTOM(glVertex2fv);
    RETURN_SYMBOL_CUSTOM(glVertex2iv);
    RETURN_SYMBOL_CUSTOM(glVertex2sv);

    RETURN_SYMBOL_CUSTOM(glVertex3dv);
    RETURN_SYMBOL_CUSTOM(glVertex3fv);
    RETURN_SYMBOL_CUSTOM(glVertex3iv);
    RETURN_SYMBOL_CUSTOM(glVertex3sv);

    RETURN_SYMBOL_CUSTOM(glVertex4dv);
    RETURN_SYMBOL_CUSTOM(glVertex4fv);
    RETURN_SYMBOL_CUSTOM(glVertex4iv);
    RETURN_SYMBOL_CUSTOM(glVertex4sv);

    RETURN_SYMBOL_CUSTOM(glDrawArrays)
    RETURN_SYMBOL_CUSTOM(glDrawElements)
    RETURN_SYMBOL_CUSTOM(glMultiDrawArrays)
    RETURN_SYMBOL_CUSTOM(glMultiDrawElements)

    RETURN_SYMBOL_CUSTOM(glDrawRangeElements);
    RETURN_SYMBOL_CUSTOM(glDrawElementsBaseVertex);
    RETURN_SYMBOL_CUSTOM(glDrawRangeElementsBaseVertex);
    RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertex);
    RETURN_SYMBOL_CUSTOM(glMultiDrawElementsBaseVertex);
    RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedBaseInstance);
    RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseInstance);
    RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedBaseVertexBaseInstance);

    RETURN_SYMBOL_CUSTOM(glDrawTransformFeedback);
    RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStream);
    RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackInstanced);
    RETURN_SYMBOL_CUSTOM(glDrawTransformFeedbackStreamInstanced);

    RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedARB);
    RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedARB);
    RETURN_SYMBOL_CUSTOM(glDrawArraysInstancedEXT);
    RETURN_SYMBOL_CUSTOM(glDrawElementsInstancedEXT);
    RETURN_SYMBOL_CUSTOM(glDrawRangeElementsEXT);
    RETURN_SYMBOL_CUSTOM(glMultiDrawArraysEXT);
    RETURN_SYMBOL_CUSTOM(glMultiDrawElementsEXT);
    RETURN_SYMBOL_CUSTOM(glDrawArraysEXT);

    RETURN_SYMBOL_CUSTOM(glBlitFramebuffer);

    RETURN_SYMBOL_CUSTOM(glTexParameterf);
    RETURN_SYMBOL_CUSTOM(glTexParameteri);
    // RETURN_SYMBOL_CUSTOM(glTexParameterfv);
    // RETURN_SYMBOL_CUSTOM(glTexParameteriv);
    // RETURN_SYMBOL_CUSTOM(glTexParameterIiv);
    // RETURN_SYMBOL_CUSTOM(glTexParameterIuiv);

    RETURN_SYMBOL_CUSTOM(glEnable);
    // RETURN_SYMBOL_CUSTOM(glDisable);
    RETURN_SYMBOL_CUSTOM(glViewport);

    return real_pointer;
}

void(*glXGetProcAddress (const GLubyte *procName))()
{
    LOG(LL_TRACE, LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(glXGetProcAddress, "GL");

    if (!orig::glXGetProcAddress) return nullptr;

    gl_load_procs(reinterpret_cast<GLGetProcAddressProc>(orig::glXGetProcAddress));

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddress(procName))));
}

__GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName)
{
    LOG(LL_TRACE, LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(glXGetProcAddressARB, "GL");

    if (!orig::glXGetProcAddressARB) return nullptr;

    gl_load_procs(reinterpret_cast<GLGetProcAddressProc>(orig::glXGetProcAddressARB));

    return reinterpret_cast<__GLXextFuncPtr>(store_orig_and_return_my_symbol(procName, reinterpret_cast<void*>(orig::glXGetProcAddressARB(procName))));
}

void* glXGetProcAddressEXT (const GLubyte *procName)
{
    LOG(LL_TRACE, LCF_OGL, "%s call with symbol %s", __func__, procName);
    LINK_NAMESPACE(glXGetProcAddressEXT, "GL");

    if (!orig::glXGetProcAddressEXT) return nullptr;

    gl_load_procs(reinterpret_cast<GLGetProcAddressProc>(orig::glXGetProcAddressEXT));

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddressEXT(procName));
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
    /* Load all GL functions here, in case the game links directly to GL
     * functions without using glXGetProcAddress() */
    LINK_NAMESPACE(glXGetProcAddress, "GL");
    gl_load_procs(reinterpret_cast<GLGetProcAddressProc>(orig::glXGetProcAddress));

    LINK_GL_POINTER(XMakeCurrent);

    Bool ret = glProcs.XMakeCurrent(dpy, drawable, ctx);
    if (GlobalState::isNative())
        return ret;

    LOGTRACE(LCF_WINDOW | LCF_OGL);

    if (drawable && XlibGameWindow::get()) {
        Global::game_info.video |= GameInfo::OPENGL;
        Global::game_info.tosend = true;

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
    /* Load all GL functions here, in case the game links directly to GL
     * functions without using glXGetProcAddress() */
    LINK_NAMESPACE(glXGetProcAddress, "GL");
    gl_load_procs(reinterpret_cast<GLGetProcAddressProc>(orig::glXGetProcAddress));

    LINK_GL_POINTER(XMakeContextCurrent);

    Bool ret = glProcs.XMakeContextCurrent(dpy, draw, read, ctx);
    if (GlobalState::isNative())
        return ret;

    LOGTRACE(LCF_WINDOW | LCF_OGL);

    if (draw && XlibGameWindow::get()) {
        Global::game_info.video |= GameInfo::OPENGL;
        Global::game_info.tosend = true;

        checkMesa();
    }

    return ret;
}

void glXSwapBuffers( Display *dpy, XID drawable )
{
    LINK_GL_POINTER(XSwapBuffers);

    if (GlobalState::isNative())
        return glProcs.XSwapBuffers(dpy, drawable);

    LOGTRACE(LCF_WINDOW | LCF_OGL);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_GL renderHUD;
    frameBoundary([&] () {glProcs.XSwapBuffers(dpy, drawable);}, renderHUD);
}

static int swapInterval = 0;

void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval)
{
    LOG(LL_TRACE, LCF_OGL, "%s call with interval %d", __func__, interval);
    LINK_GL_POINTER(XSwapIntervalEXT);

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        glProcs.XSwapIntervalEXT(dpy, drawable, interval);
    }
    else {
        glProcs.XSwapIntervalEXT(dpy, drawable, 0);
    }

}

int glXSwapIntervalSGI (int interval)
{
    LOG(LL_TRACE, LCF_OGL, "%s call with interval %d", __func__, interval);
    LINK_GL_POINTER(XSwapIntervalSGI);

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return glProcs.XSwapIntervalSGI(interval);
    }

    /* Cave Story + does use glXSwapIntervalSGI() without checking if the
     * extension is available. OpenGL software renderer llvmpipe does not
     * support this extension, and for some reasons it leads to a crash with
     * the following error:
     *      X Error of failed request:  GLXBadContextTag
     *      Major opcode of failed request:  152 (GLX)
     *      Minor opcode of failed request:  16 (X_GLXVendorPrivate)
     *
     * We can noop this call when using software renderer, which fix the issue
     * and will probably not alter much the behaviour of games. 
     */
    if (Global::shared_config.opengl_soft) {
        return 0;
    }

    int ret = glProcs.XSwapIntervalSGI(1);
    LOG(LL_DEBUG, LCF_OGL, "   ret %d", ret);
    return ret;
}

int glXSwapIntervalMESA (unsigned int interval)
{
    LOG(LL_TRACE, LCF_OGL, "%s call with interval %d", __func__, interval);
    LINK_GL_POINTER(XSwapIntervalMESA);

    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (Global::shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        return glProcs.XSwapIntervalMESA(interval);
    }
    else {
        return glProcs.XSwapIntervalMESA(0);
    }
}

int glXGetSwapIntervalMESA(void)
{
    LOGTRACE(LCF_WINDOW | LCF_OGL);
    return swapInterval;
}

const char* glXQueryExtensionsString(Display* dpy, int screen)
{
    LOGTRACE(LCF_OGL);

    /* Unity has different behaviors depending on the GLX extensions present,
     * at least for GLX_SGI_swap_control. Returning an empty string works fine. */
    if (UnityHacks::isUnity())
        return "GLX_ARB_create_context GLX_ARB_create_context_profile \
GLX_ARB_create_context_robustness GLX_ARB_fbconfig_float \
GLX_ARB_framebuffer_sRGB GLX_ARB_get_proc_address GLX_ARB_multisample \
GLX_EXT_create_context_es2_profile GLX_EXT_create_context_es_profile \
GLX_EXT_fbconfig_packed_float GLX_EXT_framebuffer_sRGB \
GLX_EXT_import_context GLX_EXT_texture_from_pixmap GLX_EXT_visual_info \
GLX_EXT_visual_rating GLX_MESA_copy_sub_buffer GLX_MESA_query_renderer \
GLX_OML_swap_method GLX_SGIS_multisample GLX_SGIX_fbconfig \
GLX_SGIX_pbuffer GLX_SGIX_visual_select_group GLX_SGI_make_current_read";

    LINK_GL_POINTER(XQueryExtensionsString);
    return glProcs.XQueryExtensionsString(dpy, screen);
}
    
void glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value)
{
    LOGTRACE(LCF_WINDOW | LCF_OGL);

    if (attribute == GLX_SWAP_INTERVAL_EXT) {
        *value = swapInterval;
        return;
    }
    if (attribute == GLX_MAX_SWAP_INTERVAL_EXT) {
        *value = 1;
        return;
    }

    LINK_GL_POINTER(XQueryDrawable);
    return glProcs.XQueryDrawable(dpy, draw, attribute, value);
}

GLXContext glXCreateContextAttribsARB (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list)
{
    LOGTRACE(LCF_OGL);
    LINK_GL_POINTER(XCreateContextAttribsARB);
    int i = 0;
    while (attrib_list[i] != 0) {
        if (attrib_list[i] == GLX_CONTEXT_MAJOR_VERSION_ARB) {
            Global::game_info.opengl_major = attrib_list[i+1];
            Global::game_info.tosend = true;
        }
        if (attrib_list[i] == GLX_CONTEXT_MINOR_VERSION_ARB) {
            Global::game_info.opengl_minor = attrib_list[i+1];
            Global::game_info.tosend = true;
        }
        if (attrib_list[i] == GLX_CONTEXT_PROFILE_MASK_ARB) {
            switch(attrib_list[i+1]) {
            case GLX_CONTEXT_CORE_PROFILE_BIT_ARB:
                Global::game_info.opengl_profile = GameInfo::CORE;
                break;
            case GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB:
                Global::game_info.opengl_profile = GameInfo::COMPATIBILITY;
                break;
            case GLX_CONTEXT_ES_PROFILE_BIT_EXT:    
                Global::game_info.opengl_profile = GameInfo::ES;
                break;
            default:
                break;
            }
            Global::game_info.tosend = true;
        }

        i += 2;
    }
    return glProcs.XCreateContextAttribsARB (dpy, config, share_context, direct, attrib_list);
}

void glXDestroyContext(Display * dpy,  GLXContext ctx)
{
    LOGTRACE(LCF_WINDOW | LCF_OGL);
    LINK_GL_POINTER(XDestroyContext);
    ScreenCapture::fini();
    return glProcs.XDestroyContext(dpy, ctx);
}

}
