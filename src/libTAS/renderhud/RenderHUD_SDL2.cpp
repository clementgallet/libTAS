/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

DEFINE_ORIG_POINTER(SDL_CreateRGBSurfaceFrom)
DEFINE_ORIG_POINTER(SDL_RenderCopy)
DEFINE_ORIG_POINTER(SDL_CreateTextureFromSurface)
DEFINE_ORIG_POINTER(SDL_GetRendererOutputSize)

RenderHUD_SDL2::~RenderHUD_SDL2()
{
}

void RenderHUD_SDL2::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

void RenderHUD_SDL2::box(int& x, int& y, int& width, int& height)
{
    x = y = 0;
    LINK_NAMESPACE_SDL2(SDL_GetRendererOutputSize);
    orig::SDL_GetRendererOutputSize(renderer, &width, &height);
}

void RenderHUD_SDL2::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    LINK_NAMESPACE_SDL2(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL2(SDL_CreateTextureFromSurface);
    LINK_NAMESPACE_SDL2(SDL_RenderCopy);

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);
    SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture* tex = orig::SDL_CreateTextureFromSurface(renderer, sdlsurf);

    SDL_Rect rect = {x, y, sdlsurf->w, sdlsurf->h};
    orig::SDL_RenderCopy(renderer, tex, NULL, &rect);
}

}

#endif
