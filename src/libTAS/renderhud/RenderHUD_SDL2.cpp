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

//#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD_SDL2.h"
#include "../logging.h"
#include "../hook.h"

int (*SDL_RenderCopy_real)(SDL_Renderer* renderer, void* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
void* (*SDL_CreateTextureFromSurface_real)(SDL_Renderer* renderer, SDL_Surface*  surface);

RenderHUD_SDL2::~RenderHUD_SDL2()
{
}

void RenderHUD_SDL2::init(void)
{
    LINK_SUFFIX_SDL2(SDL_RenderCopy);
    LINK_SUFFIX_SDL2(SDL_CreateTextureFromSurface);
    RenderHUD::init();
}

void RenderHUD_SDL2::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

void RenderHUD_SDL2::renderText(const char* text, SDL_Color fg_color, SDL_Color bg_color, int x, int y)
{
    static int inited = 0;
    if (inited == 0) {
        init();
        inited = 1;
    }

    SDL_Surface* surf = createTextSurface(text, fg_color, bg_color);
    void* tex = SDL_CreateTextureFromSurface_real(renderer, surf);

    SDL_Rect rect = {x, y, x+surf->w, y+surf->h};
    SDL_RenderCopy_real(renderer, tex, NULL, &rect);

    destroyTextSurface();
}

