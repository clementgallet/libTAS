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

#ifndef LIBTAS_EGLWRAPPERS_H_INCL
#define LIBTAS_EGLWRAPPERS_H_INCL

#include "global.h"
#include <EGL/egl.h>

namespace libtas {

OVERRIDE void(*eglGetProcAddress (const char *procName))();

/* Map the GLX context to the Display connection */
OVERRIDE EGLBoolean eglMakeCurrent( EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx );

/* Swap buffers */
OVERRIDE EGLBoolean eglSwapBuffers( EGLDisplay dpy, EGLSurface surface );

/* Set the VSync value */
OVERRIDE EGLBoolean eglSwapInterval (EGLDisplay dpy, EGLint interval);

OVERRIDE EGLBoolean eglBindAPI(EGLenum api);

OVERRIDE EGLContext eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);

}

#endif
