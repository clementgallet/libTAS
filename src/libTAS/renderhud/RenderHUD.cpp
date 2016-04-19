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

#include "RenderHUD.h"
#include "../logging.h"
#include "../hook.h"

const char* fontpath = "/home/clement/libTAS/src/external/GenBkBasR.ttf";

int (*SDL_SetSurfaceBlendMode_real)(SDL_Surface* surface, SDL_BlendMode blendMode);
int (*SDL_UpperBlit_real)(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst,SDL_Rect* dstrect);

RenderHUD::RenderHUD()
{
    outline_size = 1;
    font_size = 20;
}

RenderHUD::~RenderHUD()
{
    if (fg_font)
        TTF_CloseFont(fg_font);
    if (bg_font)
        TTF_CloseFont(bg_font);
    if (TTF_WasInit())
        TTF_Quit();
}

void RenderHUD::init()
{
    init(fontpath);
}

void RenderHUD::init(const char* path)
{
    LINK_SUFFIX_SDL2(SDL_SetSurfaceBlendMode);
    LINK_SUFFIX_SDL2(SDL_UpperBlit);

    /* Initialize SDL TTF */
    if(TTF_Init() == -1) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't init SDL TTF.");
        return;
    }

    fg_font = TTF_OpenFont(path, 30);
    if (fg_font == NULL) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't load font");
        return;
    }

    bg_font = TTF_OpenFont(path, 30);
    if (bg_font == NULL) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't load font");
        return;
    }

    TTF_SetFontOutline(bg_font, outline_size);
}

SurfaceARGB* RenderHUD::createTextSurface(const char* text, SDL_Color fg_color, SDL_Color bg_color)
{
    fg_surf = TTF_RenderText_Blended(fg_font, text, fg_color);
    bg_surf = TTF_RenderText_Blended(bg_font, text, bg_color);

    /* Blit text onto its outline. */
    bg_surf->blit(fg_surf, outline_size, outline_size);
    delete fg_surf;

    return bg_surf;
}

void RenderHUD::destroyTextSurface()
{
    delete bg_surf;
}

#endif

