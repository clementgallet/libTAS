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

#include "RenderHUD_SDL1.h"
#ifdef LIBTAS_ENABLE_HUD

#include <SDL2/SDL.h> // SDL_Rect
#include "../../external/SDL1.h" // SDL1::SDL_Surface
#include "../logging.h"
#include "../hook.h"

namespace libtas {

namespace orig {
    static SDL1::SDL_Surface *(*SDL_CreateRGBSurfaceFrom)(void *pixels,
            int width, int height, int depth, int pitch,
            Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    static void (*SDL_FreeSurface)(SDL1::SDL_Surface *surface);
    static SDL1::SDL_Surface *(*SDL_GetVideoSurface)(void);
    static int (*SDL_UpperBlit)(SDL1::SDL_Surface *src, SDL1::SDL_Rect *srcrect, SDL1::SDL_Surface *dst, SDL1::SDL_Rect *dstrect);
    static uint8_t (*SDL_SetClipRect)(SDL1::SDL_Surface *surface, const SDL1::SDL_Rect *rect);
    static void (*SDL_GetClipRect)(SDL1::SDL_Surface *surface, SDL1::SDL_Rect *rect);
}

RenderHUD_SDL1::~RenderHUD_SDL1()
{
}

void RenderHUD_SDL1::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    LINK_NAMESPACE_SDL1(SDL_CreateRGBSurfaceFrom);
    LINK_NAMESPACE_SDL1(SDL_FreeSurface);
    LINK_NAMESPACE_SDL1(SDL_GetVideoSurface);
    LINK_NAMESPACE_SDL1(SDL_UpperBlit);
    LINK_NAMESPACE_SDL1(SDL_SetClipRect);
    LINK_NAMESPACE_SDL1(SDL_GetClipRect);

    GlobalNative gn;

    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);
    
    if (!surf)
        return;
        
    SDL1::SDL_Surface* sdlsurf = orig::SDL_CreateRGBSurfaceFrom(surf->pixels.data(), surf->w, surf->h, 32, surf->pitch, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    SDL1::SDL_Surface* screen = orig::SDL_GetVideoSurface();

    /* Change the coords so that the text fills on screen */
    x = (x + surf->w + 5) > screen->w ? (screen->w - surf->w - 5) : x;
    y = (y + surf->h + 5) > screen->h ? (screen->h - surf->h - 5) : y;

    SDL1::SDL_Rect rect = {static_cast<Sint16>(x), static_cast<Sint16>(y), 0, 0}; // width and height are ignored

    /* Save and restore the clip rectangle */
    SDL1::SDL_Rect clip_rect;
    orig::SDL_GetClipRect(screen, &clip_rect);
    orig::SDL_SetClipRect(screen, nullptr);
    orig::SDL_UpperBlit(sdlsurf, NULL, screen, &rect);
    orig::SDL_SetClipRect(screen, &clip_rect);

    orig::SDL_FreeSurface(sdlsurf);
}

}

#endif
