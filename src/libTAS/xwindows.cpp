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

/*
 * Store the game Window identifier
 * We assume the game never open multiple windows at a time
 */
Window gameXWindow = 0;

/* Has the game window pointer be sent to the program? */
static bool gw_sent = false;

// #ifdef LIBTAS_ENABLE_AVDUMPING
// std::unique_ptr<AVEncoder> avencoder;
// #endif
//
// bool video_opengl = false;

/* Path of the dump file */
// char av_filename[4096] = {0};

namespace orig {
    static void (*glXSwapBuffers)( Display *dpy, XID drawable );
    static Window (*XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);
    static Window (*XCreateSimpleWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background);
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

    gameXWindow = orig::XCreateWindow(display, parent, x, y, width, height, border_width, depth, klass, visual, valuemask, attributes);

    sendMessage(MSGB_WINDOW_ID);
    sendData(&gameXWindow, sizeof(Window));
    gw_sent = true;
    debuglog(LCF_WINDOW, "Sent X11 window id: ", gameXWindow);

    return gameXWindow;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XCreateSimpleWindow, nullptr);

    gameXWindow = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);

    sendMessage(MSGB_WINDOW_ID);
    sendData(&gameXWindow, sizeof(Window));
    gw_sent = true;
    debuglog(LCF_WINDOW, "Sent X11 window id: ", gameXWindow);

    return gameXWindow;
}

}
