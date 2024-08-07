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

#ifndef LIBTAS_RENDERHUD_GL_H_INCL
#define LIBTAS_RENDERHUD_GL_H_INCL

#ifdef __unix__
#include "RenderHUD_Base_Linux.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "RenderHUD_Base_MacOS.h"
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

        /* Destroy context */
        static void fini();

        /* Switch renderer to OpenGL ES */
        void setGLES(bool stateGLES) {isGLES = stateGLES;}

        void newFrame();

        void endFrame();

        void render();

        /* Does the backend supports rendering the game inside an ImGui window? */
        bool supportsGameWindow() {return true;}

        /* Does the backend supports extending the window size to use it as
         * a working area when game window is detached? */
        bool supportsLargerViewport() {return true;}

        bool invertedOrigin() {return true;}

    private:        
        bool isGLES = false;
};
}

#endif
