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
//#include "screenpixels.h"

namespace libtas {

namespace orig {
    static void (*glXSwapBuffers)( Display *dpy, XID drawable );
    static Window (*XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);
    static Window (*XCreateSimpleWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background);
    static int (*XMapWindow)(Display *display, Window w);
    static int (*XMapRaised)(Display *display, Window w);
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
