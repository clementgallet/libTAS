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

#include "RenderHUD_SDL2_surface.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"
#include "../sdl/sdlwindows.h" // sdl::gameSDLWindow
#include "../ScreenCapture.h"

#include <SDL2/SDL.h>

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateRGBSurfaceFrom)
DECLARE_ORIG_POINTER(SDL_GetWindowSurface)
DECLARE_ORIG_POINTER(SDL_UpperBlit)

RenderHUD_SDL2_surface::~RenderHUD_SDL2_surface()
{
}

void RenderHUD_SDL2_surface::renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y)
{
    LINK_NAMESPACE_SDL2(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL2(SDL_GetWindowSurface);
    LINK_NAMESPACE_SDL2(SDL_UpperBlit);

    GlobalNative gn;

    SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Surface* screensurf = orig::SDL_GetWindowSurface(sdl::gameSDLWindow);

    /* Change the coords so that the text fills on screen */
    int width, height;
    ScreenCapture::getDimensions(width, height);
    x = (x + surf->w + 5) > width ? (width - surf->w - 5) : x;
    y = (y + surf->h + 5) > height ? (height - surf->h - 5) : y;

    SDL_Rect rect = {x, y, sdlsurf->w, sdlsurf->h};
    orig::SDL_UpperBlit(sdlsurf, NULL, screensurf, &rect);
}

}

#endif
