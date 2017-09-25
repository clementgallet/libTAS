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

#ifndef LIBTAS_XWINDOWS_H_INCL
#define LIBTAS_XWINDOWS_H_INCL

#include "global.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

namespace libtas {

OVERRIDE void* glXGetProcAddress (const GLubyte *procName);
OVERRIDE void* glXGetProcAddressARB (const GLubyte *procName);
OVERRIDE void* glXGetProcAddressEXT (const GLubyte *procName);

/* Map the GLX context to the Display connection */
OVERRIDE Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx );

/* Swap buffers */
OVERRIDE void glXSwapBuffers( Display *dpy, GLXDrawable drawable );

/* Set the VSync value */
OVERRIDE void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval);
OVERRIDE int glXSwapIntervalSGI (int interval);
OVERRIDE int glXSwapIntervalMESA (unsigned int interval);

OVERRIDE int glXGetSwapIntervalMESA(void);

/* Returns an attribute assoicated with a GLX drawable */
OVERRIDE int glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value);

OVERRIDE Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);

OVERRIDE Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background);

OVERRIDE int XDestroyWindow(Display *display, Window w);

OVERRIDE int XMapWindow(Display *display, Window w);

OVERRIDE int XMapRaised(Display *display, Window w);
}

#endif
