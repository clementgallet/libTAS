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

#ifndef LIBTAS_GLXWRAPPERS_H_INCL
#define LIBTAS_GLXWRAPPERS_H_INCL

#include "global.h"
#define GL_GLEXT_PROTOTYPES
// #include <GL/gl.h>
#include <GL/glx.h>
// #include <GL/glext.h>
#include <GL/glxext.h>

namespace libtas {

void checkMesa();

OVERRIDE void(*glXGetProcAddress (const GLubyte *procName))();
OVERRIDE __GLXextFuncPtr glXGetProcAddressARB (const GLubyte *procName);
OVERRIDE void* glXGetProcAddressEXT (const GLubyte *procName);

/* Map the GLX context to the Display connection */
OVERRIDE Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx );

/* Attach a GLX context to a GLX drawable */
OVERRIDE Bool glXMakeContextCurrent( Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx );

/* Swap buffers */
OVERRIDE void glXSwapBuffers( Display *dpy, GLXDrawable drawable );

/* Set the VSync value */
OVERRIDE void glXSwapIntervalEXT (Display *dpy, GLXDrawable drawable, int interval);
OVERRIDE int glXSwapIntervalSGI (int interval);
OVERRIDE int glXSwapIntervalMESA (unsigned int interval);

OVERRIDE int glXGetSwapIntervalMESA(void);

OVERRIDE const char* glXQueryExtensionsString(Display* dpy, int screen);

/* Returns an attribute assoicated with a GLX drawable */
OVERRIDE void glXQueryDrawable(Display * dpy,  GLXDrawable draw,  int attribute,  unsigned int * value);

OVERRIDE GLXContext glXCreateContextAttribsARB (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
OVERRIDE void glXDestroyContext(Display * dpy,  GLXContext ctx);

}

#endif
