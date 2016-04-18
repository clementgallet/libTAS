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

#ifndef LIBTAS_RENDERHUD_H_INCL
#define LIBTAS_RENDERHUD_H_INCL

#include "../../external/SDL.h"
#include "sdl_ttf.h"

class RenderHUD
{
    public:
        RenderHUD();
        virtual ~RenderHUD();

        void init();
        void init(const char* path);

        virtual void renderText(const char* text, SDL_Color fg_color, SDL_Color bg_color, int x, int y) = 0;

    protected:
        SDL_Surface* createTextSurface(const char* text, SDL_Color fg_color, SDL_Color bg_color);
        void destroyTextSurface();

    private:
        int outline_size;
        int font_size;

        TTF_Font* fg_font;
        TTF_Font* bg_font;

        SDL_Surface* fg_surf;
        SDL_Surface* bg_surf;
};

#endif

