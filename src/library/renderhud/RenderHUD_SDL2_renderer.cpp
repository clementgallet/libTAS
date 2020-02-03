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

#include "RenderHUD_SDL2_renderer.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"
#include "../ScreenCapture.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateRGBSurfaceFrom)
DECLARE_ORIG_POINTER(SDL_RenderCopy)
DECLARE_ORIG_POINTER(SDL_CreateTextureFromSurface)

RenderHUD_SDL2_renderer::~RenderHUD_SDL2_renderer()
{
}

void RenderHUD_SDL2_renderer::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

void RenderHUD_SDL2_renderer::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    LINK_NAMESPACE_SDL2(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL2(SDL_CreateTextureFromSurface);
    LINK_NAMESPACE_SDL2(SDL_RenderCopy);

    GlobalNative gn;

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);
    SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture* tex = orig::SDL_CreateTextureFromSurface(renderer, sdlsurf);

    /* Change the coords so that the text fills on screen */
    int width, height;
    ScreenCapture::getDimensions(width, height);
    x = (x + surf->w + 5) > width ? (width - surf->w - 5) : x;
    y = (y + surf->h + 5) > height ? (height - surf->h - 5) : y;

    SDL_Rect rect = {x, y, sdlsurf->w, sdlsurf->h};
    orig::SDL_RenderCopy(renderer, tex, NULL, &rect);
}

}

#endif
