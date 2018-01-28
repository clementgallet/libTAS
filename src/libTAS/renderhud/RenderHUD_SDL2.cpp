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

#include "RenderHUD_SDL2.h"
#include "../logging.h"
#include "../hook.h"

namespace libtas {

namespace orig {
    static SDL_Surface *(*SDL_CreateRGBSurfaceFrom)(void *pixels,
            int width, int height, int depth, int pitch,
            Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    static int (*SDL_RenderCopy)(SDL_Renderer* renderer, void* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
    static void* (*SDL_CreateTextureFromSurface)(SDL_Renderer* renderer, SDL_Surface*  surface);
    static int (*SDL_GetRendererOutputSize)(SDL_Renderer* renderer, int* w, int* h);
}

RenderHUD_SDL2::~RenderHUD_SDL2()
{
}

void RenderHUD_SDL2::init(void)
{
    LINK_NAMESPACE_SDL2(SDL_RenderCopy);
    LINK_NAMESPACE_SDL2(SDL_CreateTextureFromSurface);
    LINK_NAMESPACE_SDL2(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL2(SDL_GetRendererOutputSize);
    RenderHUD::init();
}

void RenderHUD_SDL2::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

void RenderHUD_SDL2::box(int& x, int& y, int& width, int& height)
{
    x = y = 0;
    orig::SDL_GetRendererOutputSize(renderer, &width, &height);
}

void RenderHUD_SDL2::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    static int inited = 0;
    if (inited == 0) {
        init();
        inited = 1;
    }

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);
    SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    void* tex = orig::SDL_CreateTextureFromSurface(renderer, sdlsurf);

    SDL_Rect rect = {x, y, x+sdlsurf->w, y+sdlsurf->h};
    orig::SDL_RenderCopy(renderer, tex, NULL, &rect);
}

}

#endif
