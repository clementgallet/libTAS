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

#include "sdlrender.h"
#include "sdlwindows.h"
#include "hook.h"
#include "logging.h"
#include "frame.h"
#include "renderhud/RenderHUD_SDL2_renderer.h"
#include "ScreenCapture.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateRenderer);
DECLARE_ORIG_POINTER(SDL_DestroyRenderer);
DECLARE_ORIG_POINTER(SDL_RenderPresent);
DECLARE_ORIG_POINTER(SDL_RenderSetViewport);
DECLARE_ORIG_POINTER(SDL_RenderGetViewport);
DECLARE_ORIG_POINTER(SDL_RenderSetScale);
DECLARE_ORIG_POINTER(SDL_RenderGetScale);

/* Override */ SDL_Renderer *SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_CreateRenderer);

    if (flags & SDL_RENDERER_SOFTWARE)
        debuglog(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_SOFTWARE");
    if (flags & SDL_RENDERER_ACCELERATED)
        debuglog(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_ACCELERATED");
    if (flags & SDL_RENDERER_PRESENTVSYNC)
        debuglog(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_PRESENTVSYNC");
    if (flags & SDL_RENDERER_TARGETTEXTURE)
        debuglog(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_TARGETTEXTURE");

    game_info.video |=  GameInfo::SDL2_RENDERER;

    SDL_Renderer* renderer = orig::SDL_CreateRenderer(window, index, flags);

    ScreenCapture::init();

    return renderer;
}

/* Override */ void SDL_DestroyRenderer(SDL_Renderer * renderer)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_DestroyRenderer);

    ScreenCapture::fini();

    orig::SDL_DestroyRenderer(renderer);
}

/* Override */ void SDL_RenderPresent(SDL_Renderer * renderer)
{
    LINK_NAMESPACE_SDL2(SDL_RenderPresent);

    if (GlobalState::isNative())
        return orig::SDL_RenderPresent(renderer);

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL2_renderer renderHUD;
    renderHUD.setRenderer(renderer);
    frameBoundary(true, [&] () {orig::SDL_RenderPresent(renderer);}, renderHUD);
#else
    frameBoundary(true, [&] () {orig::SDL_RenderPresent(renderer);});
#endif
}

static int logical_w = 0;
static int logical_h = 0;

int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " called with new size: ", w, " x ", h);

    /* Don't let the game have logical size that differs from screen size,
     * so we resize the window instead.
     */
    SDL_SetWindowSize(gameSDLWindow, w, h);
    logical_w = w;
    logical_h = h;

    return 0;
}

void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Set the stored values of logical size */
    *w = logical_w;
    *h = logical_h;
}

int SDL_RenderSetViewport(SDL_Renderer * renderer, const SDL_Rect * rect)
{
    if (rect)
        debuglog(LCF_SDL | LCF_WINDOW | LCF_TODO, __func__, " called with new size: ", rect->w, " x ", rect->h);
    else
        debuglog(LCF_SDL | LCF_WINDOW | LCF_TODO, __func__, " called with native size");

    LINK_NAMESPACE_SDL2(SDL_RenderSetViewport);
    return orig::SDL_RenderSetViewport(renderer, rect);
}

void SDL_RenderGetViewport(SDL_Renderer * renderer, SDL_Rect * rect)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW | LCF_TODO);
    LINK_NAMESPACE_SDL2(SDL_RenderGetViewport);
    return orig::SDL_RenderGetViewport(renderer, rect);
}

int SDL_RenderSetScale(SDL_Renderer * renderer, float scaleX, float scaleY)
{
    debuglog(LCF_SDL | LCF_WINDOW | LCF_TODO, __func__, " called with new scaleX: ", scaleX, " and scaleY: ", scaleY);
    LINK_NAMESPACE_SDL2(SDL_RenderSetScale);
    return orig::SDL_RenderSetScale(renderer, scaleX, scaleY);
}

void SDL_RenderGetScale(SDL_Renderer * renderer, float *scaleX, float *scaleY)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW | LCF_TODO);
    LINK_NAMESPACE_SDL2(SDL_RenderGetScale);
    return orig::SDL_RenderGetScale(renderer, scaleX, scaleY);
}

}
