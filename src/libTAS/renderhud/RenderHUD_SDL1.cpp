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
#include <SDL2/SDL.h> // SDL_Rect
#include "../../external/SDL1.h" // SDL1::SDL_Surface
#include "../logging.h"
#include "../hook.h"

namespace libtas {

namespace orig {
    static SDL1::SDL_Surface *(*SDL_CreateRGBSurfaceFrom)(void *pixels,
            int width, int height, int depth, int pitch,
            Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    static SDL1::SDL_Surface *(*SDL_GetVideoSurface)(void);
    static int (*SDL_BlitSurface)(SDL1::SDL_Surface *src, SDL1::SDL_Rect *srcrect, SDL1::SDL_Surface *dst, SDL1::SDL_Rect *dstrect);
}

RenderHUD_SDL1::~RenderHUD_SDL1()
{
}

void RenderHUD_SDL1::box(int& x, int& y, int& width, int& height)
{
    x = y = 0;
    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    SDL1::SDL_Surface* screen = orig::SDL_GetVideoSurface();
    width = screen->w;
    height = screen->h;
}

void RenderHUD_SDL1::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    LINK_NAMESPACE_SDL1(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    LINK_NAMESPACE_SDL1(SDL_BlitSurface);

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);
    SDL1::SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    SDL1::SDL_Surface* screen = orig::SDL_GetVideoSurface();

    SDL1::SDL_Rect rect = {static_cast<Sint16>(x), static_cast<Sint16>(y), 0, 0}; // width and height are ignored
    orig::SDL_BlitSurface(sdlsurf, NULL, screen, &rect);
}

}

#endif
