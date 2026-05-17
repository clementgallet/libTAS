/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "sdldynapi.h"

#include "logging.h"
#include "frame.h"
#include "renderhud/RenderHUD_SDL2_renderer.h"
#include "screencapture/ScreenCapture.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

namespace sdl2 {

SDL_Renderer *SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW);

    if (flags & sdl2::SDL_RENDERER_SOFTWARE)
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_SOFTWARE");
    if (flags & sdl2::SDL_RENDERER_ACCELERATED)
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_ACCELERATED");
    if (flags & sdl2::SDL_RENDERER_PRESENTVSYNC)
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_PRESENTVSYNC");
    if (flags & sdl2::SDL_RENDERER_TARGETTEXTURE)
        LOG(LL_DEBUG, LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_TARGETTEXTURE");

    Global::game_info.video |=  GameInfo::SDL2_RENDERER;

    SDL_Renderer* renderer = ORIG_SDL2_CALL(SDL_CreateRenderer, (window, index, flags));

    return renderer;
}

};

/* Override */ void SDL_DestroyRenderer(SDL_Renderer * renderer)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW);

    ScreenCapture::fini();

    Global::game_info.video &= ~GameInfo::SDL2_RENDERER;

    ORIG_SDL2_CALL(SDL_DestroyRenderer, (renderer));
}

/* Override */ void SDL_RenderPresent(SDL_Renderer * renderer)
{
    if (GlobalState::isNative())
        return ORIG_SDL23_CALL(SDL_RenderPresent, (renderer));

    LOGTRACE(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_SDL2_renderer renderHUD;
    renderHUD.setRenderer(renderer);
    frameBoundary([&] () {ORIG_SDL23_CALL(SDL_RenderPresent, (renderer));}, renderHUD);
}

static int logical_w = 0;
static int logical_h = 0;

int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW, "%s called with new size: %d x %d", __func__, w, h);

    /* Don't let the game have logical size that differs from screen size,
     * so we resize the window instead.
     */
    if (sdl::gameSDLWindow) {
        libtas::SDL_SetWindowSize(sdl::gameSDLWindow, w, h);
    }
    logical_w = w;
    logical_h = h;

    return 0;
}

void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW);

    /* Set the stored values of logical size */
    if (w) {
        *w = logical_w;
    }
    if (h) {
        *h = logical_h;
    }
}

int SDL_RenderSetViewport(SDL_Renderer * renderer, const sdl2::SDL_Rect * rect)
{
    if (rect)
        LOG(LL_TRACE, LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with new size: %d x %d", __func__, rect->w, rect->h);
    else
        LOG(LL_TRACE, LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with native size", __func__);

    return ORIG_SDL2_CALL(SDL_RenderSetViewport, (renderer, rect));
}

void SDL_RenderGetViewport(SDL_Renderer * renderer, sdl2::SDL_Rect * rect)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW | LCF_TODO);
    return ORIG_SDL2_CALL(SDL_RenderGetViewport, (renderer, rect));
}

int SDL_RenderSetScale(SDL_Renderer * renderer, float scaleX, float scaleY)
{
    LOG(LL_TRACE, LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with new scaleX: %d and scaleY: %d", __func__, scaleX, scaleY);
    return ORIG_SDL2_CALL(SDL_RenderSetScale, (renderer, scaleX, scaleY));
}

void SDL_RenderGetScale(SDL_Renderer * renderer, float *scaleX, float *scaleY)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW | LCF_TODO);
    return ORIG_SDL2_CALL(SDL_RenderGetScale, (renderer, scaleX, scaleY));
}

namespace sdl3 {

SDL_Renderer * SDL_CreateRenderer(SDL_Window *window, const char *name)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW);

    SDL_Renderer* renderer = ORIG_SDL3_CALL(SDL_CreateRenderer, (window, name));

    Global::game_info.video |=  GameInfo::SDL2_RENDERER;

    return renderer;
}

}

}