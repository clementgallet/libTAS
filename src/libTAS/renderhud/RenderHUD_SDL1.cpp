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

#ifdef LIBTAS_ENABLE_HUD

#include "RenderHUD_SDL1.h"
#include "../logging.h"
#include "../hook.h"

static SDL1::SDL_Surface *(*SDL_CreateRGBSurfaceFrom_real)(void *pixels,
                int width, int height, int depth, int pitch,
                Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
static SDL1::SDL_Surface *(*SDL_GetVideoSurface_real)(void);
static int (*SDL_UpperBlit_real)(SDL1::SDL_Surface *src, SDL_Rect *srcrect, SDL1::SDL_Surface *dst, SDL_Rect *dstrect);

RenderHUD_SDL1::~RenderHUD_SDL1()
{
}

void RenderHUD_SDL1::init(void)
{
    LINK_SUFFIX_SDL1(SDL_GetVideoSurface);
    LINK_SUFFIX_SDL1(SDL_UpperBlit);
    LINK_SUFFIX_SDL1(SDL_CreateRGBSurfaceFrom);
    RenderHUD::init();
}

void RenderHUD_SDL1::renderText(const char* text, SDL_Color fg_color, SDL_Color bg_color, int x, int y)
{
    static int inited = 0;
    if (inited == 0) {
        init();
        inited = 1;
    }

    SurfaceARGB* surf = createTextSurface(text, fg_color, bg_color);
    SDL1::SDL_Surface* sdlsurf = SDL_CreateRGBSurfaceFrom_real(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    SDL1::SDL_Surface* screen = SDL_GetVideoSurface_real();

    SDL_Rect rect = {x, y, x+sdlsurf->w, y+sdlsurf->h};
    SDL_UpperBlit_real(sdlsurf, NULL, screen, &rect);

    destroyTextSurface();
}

#endif

