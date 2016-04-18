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

#ifndef LIBTAS_RENDERHUD_GL_H_INCL
#define LIBTAS_RENDERHUD_GL_H_INCL

#include "RenderHUD.h"
#include "../../external/SDL.h"
#include "../../external/gl.h"

class RenderHUD_GL : public RenderHUD
{
    public:
        ~RenderHUD_GL();
        void init(void);
        void renderText(const char* text, SDL_Color fg_color, SDL_Color bg_color, int x, int y);

    private:
        void enterRender(void);
        void exitRender(void);

        GLint oldProgram;
        GLboolean oldTex2DEnabled;
        GLboolean oldBlendEnabled;
        GLint oldBlendSrc;
        GLint oldBlendDst;
        GLint oldTex;
};

#endif

