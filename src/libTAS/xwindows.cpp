/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xwindows.h"
#include "hook.h"
#include "logging.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
//#include "../shared/SharedConfig.h"
#include "frame.h"
#include "renderhud/RenderHUD_GL.h"
#include "ScreenCapture.h"

namespace libtas {

namespace orig {
    static void* (*glXGetProcAddress) (const GLubyte *procName);
    static void* (*glXGetProcAddressARB) (const GLubyte *procName);
    static void* (*glXGetProcAddressEXT) (const GLubyte *procName);
    static Bool (*glXMakeCurrent)( Display *dpy, GLXDrawable drawable, GLXContext ctx );
    static void (*glXSwapBuffers)( Display *dpy, GLXDrawable drawable );
    static void (*glXSwapIntervalEXT) (Display *dpy, GLXDrawable drawable, int interval);
    static int (*glXSwapIntervalSGI) (int interval);
    static int (*glXSwapIntervalMESA) (unsigned int interval);
    static int (*glXGetSwapIntervalMESA) (void);
    static int (*glXQueryDrawable)(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value);
    static Window (*XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);
    static Window (*XCreateSimpleWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background);
    static int (*XDestroyWindow)(Display *display, Window w);
    static int (*XMapWindow)(Display *display, Window w);
    static int (*XMapRaised)(Display *display, Window w);
}

/* If the game uses the glXGetProcAddressXXX functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* store_orig_and_return_my_symbol(const GLubyte* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    if (!strcmp(reinterpret_cast<const char*>(symbol), "glXSwapIntervalEXT")) {
        orig::glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(real_pointer);
        return reinterpret_cast<void*>(glXSwapIntervalEXT);
    }

    if (!strcmp(reinterpret_cast<const char*>(symbol), "glXSwapIntervalSGI")) {
        orig::glXSwapIntervalSGI = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(real_pointer);
        return reinterpret_cast<void*>(glXSwapIntervalSGI);
    }

    if (!strcmp(reinterpret_cast<const char*>(symbol), "glXSwapIntervalMESA")) {
        orig::glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(real_pointer);
        return reinterpret_cast<void*>(glXSwapIntervalMESA);
    }

    if (!strcmp(reinterpret_cast<const char*>(symbol), "glXGetSwapIntervalMESA")) {
        orig::glXGetSwapIntervalMESA = reinterpret_cast<PFNGLXGETSWAPINTERVALMESAPROC>(real_pointer);
        return reinterpret_cast<void*>(glXGetSwapIntervalMESA);
    }

    return real_pointer;
}

void* glXGetProcAddress (const GLubyte *procName)
{
    debuglog(LCF_WINDOW, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddress, "libGL");

    if (!orig::glXGetProcAddress) return nullptr;

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddress(procName));
}

void* glXGetProcAddressARB (const GLubyte *procName)
{
    debuglog(LCF_WINDOW, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressARB, "libGL");

    if (!orig::glXGetProcAddressARB) return nullptr;

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddressARB(procName));
}

void* glXGetProcAddressEXT (const GLubyte *procName)
{
    debuglog(LCF_WINDOW, __func__, " call with symbol ", procName);
    LINK_NAMESPACE(glXGetProcAddressEXT, "libGL");

    if (!orig::glXGetProcAddressEXT) return nullptr;

    return store_orig_and_return_my_symbol(procName, orig::glXGetProcAddressEXT(procName));
}

Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(glXMakeCurrent, "libGL");

    Bool ret = orig::glXMakeCurrent(dpy, drawable, ctx);

    if (drawable != None) {
        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;

        /* Now that the context is created, we can init the screen capture */
        ScreenCapture::init(nullptr, nullptr, nullptr);
    }

    /* Disable VSync */
    //LINK_NAMESPACE(glXGetProcAddressARB, "libGL");
    //orig::glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)orig::glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    //orig::glXSwapIntervalEXT(dpy, drawable, 0);

    return ret;
}

void glXSwapBuffers( Display *dpy, XID drawable )
{
    DEBUGLOGCALL(LCF_FRAME | LCF_WINDOW);
    LINK_NAMESPACE(glXSwapBuffers, "libGL");

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
    debuglog(LCF_WINDOW, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalEXT, "libGL");

    /* Store the interval but don't set it */
    swapInterval = interval;
    orig::glXSwapIntervalEXT(dpy, drawable, 0);
}

int glXSwapIntervalSGI (int interval)
{
    debuglog(LCF_WINDOW, __func__, " call with interval ", interval);
    LINK_NAMESPACE(glXSwapIntervalSGI, "libGL");

    /* Store the interval but don't set it */
    swapInterval = interval;
    return orig::glXSwapIntervalSGI(0);
}

int glXSwapIntervalMESA (unsigned int interval)
{
    debuglog(LCF_WINDOW, __func__, " call with interval ", interval);
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

int glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (attribute == GLX_SWAP_INTERVAL_EXT) {
        *value = swapInterval;
        return True;
    }
    if (attribute == GLX_MAX_SWAP_INTERVAL_EXT) {
        *value = 1;
        return True;
    }

    LINK_NAMESPACE(glXQueryDrawable, "libGL");
    return orig::glXQueryDrawable(dpy, draw, attribute, value);
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XCreateWindow, nullptr);

    Window w = orig::XCreateWindow(display, parent, x, y, width, height, border_width, depth, klass, visual, valuemask, attributes);

    return w;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XCreateSimpleWindow, nullptr);

    Window w = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);

    return w;
}

int XDestroyWindow(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XDestroyWindow, nullptr);
    return orig::XDestroyWindow(display, w);
}

int XMapWindow(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XMapWindow, nullptr);

    /* Only send the Window indentifier when the window is mapped, because
     * SDL does create some unmapped windows at startup.
     */
    sendMessage(MSGB_WINDOW_ID);
    sendData(&w, sizeof(Window));
    debuglog(LCF_WINDOW, "Sent X11 window id: ", w);

    gameDisplay = display;
    gameXWindow = w;

    return orig::XMapWindow(display, w);
}

int XMapRaised(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XMapRaised, nullptr);

    sendMessage(MSGB_WINDOW_ID);
    sendData(&w, sizeof(Window));
    debuglog(LCF_WINDOW, "Sent X11 window id: ", w);

    gameDisplay = display;
    gameXWindow = w;

    return orig::XMapRaised(display, w);
}

}
