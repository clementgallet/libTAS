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

#ifndef LIBTAS_RENDERHUD_GL_H_INCL
#define LIBTAS_RENDERHUD_GL_H_INCL

#ifdef __unix__
#include "RenderHUD_Base_Linux.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "RenderHUD_Base_MacOS.h"
#endif

#ifdef LIBTAS_ENABLE_HUD

#define GL_GLEXT_PROTOTYPES
#ifdef __unix__
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

namespace libtas {
#ifdef __unix__
class RenderHUD_GL : public RenderHUD_Base_Linux
#elif defined(__APPLE__) && defined(__MACH__)
class RenderHUD_GL : public RenderHUD_Base_MacOS
#endif
{
    public:
        ~RenderHUD_GL();

        /* Initialize texture and fbo */
        static void init(bool stateGLES);

        /* Deallocate texture and fbo */
        static void fini();

        /* Switch renderer to OpenGL ES */
        void setGLES(bool stateGLES) {isGLES = stateGLES;}

        void renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y);
    private:
        static GLuint texture;
        static GLuint vao;
        static GLuint vbo;
        static GLuint ebo;
        static GLuint programID;
        
        static float vertices[20];
        static GLuint indices[6];
        
        bool isGLES = false;
};
}

#endif
#endif
