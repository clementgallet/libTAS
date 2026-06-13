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
#include "renderhud/RenderHUD_SDL3_renderer.h"
#include "screencapture/ScreenCapture.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

SDL_Renderer *sdl2::SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

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

SDL_Renderer *sdl3::SDL_CreateRenderer(SDL_Window *window, const char *name)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

    SDL_Renderer* renderer = ORIG_SDL3_CALL(SDL_CreateRenderer, (window, name));

    Global::game_info.video |=  GameInfo::SDL3_RENDERER;

    return renderer;
}

/* Override */ void SDL_DestroyRenderer(SDL_Renderer * renderer)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

    ScreenCapture::fini();

    Global::game_info.video &= ~GameInfo::SDL2_RENDERER;
    Global::game_info.video &= ~GameInfo::SDL3_RENDERER;

    ORIG_SDL23_CALL(SDL_DestroyRenderer, (renderer));
}

/* Override */ void SDL_RenderPresent(SDL_Renderer * renderer)
{
    if (GlobalState::isNative())
        return ORIG_SDL23_CALL(SDL_RenderPresent, (renderer));

    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
    if (Global::game_info.video & GameInfo::SDL3_RENDERER) {
        static RenderHUD_SDL3_renderer renderHUD;
        renderHUD.setRenderer(renderer);
        frameBoundary([&] () {NATIVECALL(ORIG_SDL3_CALL(SDL_RenderPresent, (renderer)));}, renderHUD);
    }
    else {
        static RenderHUD_SDL2_renderer renderHUD;
        renderHUD.setRenderer(renderer);
        frameBoundary([&] () {NATIVECALL(ORIG_SDL2_CALL(SDL_RenderPresent, (renderer)));}, renderHUD);
    }
}

static int logical_w = 0;
static int logical_h = 0;

int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW, "%s called with new size: %d x %d", __func__, w, h);

    /* Don't let the game have logical size that differs from screen size,
     * so we resize the window instead.
     */
    if (sdl::gameSDLWindow) {
        libtas::sdl2::SDL_SetWindowSize(sdl::gameSDLWindow, w, h);
    }
    logical_w = w;
    logical_h = h;

    return 0;
}

static sdl3::SDL_RendererLogicalPresentation logical_mode = sdl3::SDL_LOGICAL_PRESENTATION_DISABLED;

bool SDL_SetRenderLogicalPresentation(SDL_Renderer *renderer, int w, int h, sdl3::SDL_RendererLogicalPresentation mode)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW, "%s called with new size: %d x %d", __func__, w, h);

    /* Don't let the game have logical size that differs from screen size,
     * so we resize the window instead.
     */
    if (sdl::gameSDLWindow) {
        libtas::sdl3::SDL_SetWindowSize(sdl::gameSDLWindow, w, h);
    }
    logical_w = w;
    logical_h = h;
    logical_mode = mode;
    return true;
}

void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

    /* Set the stored values of logical size */
    if (w) {
        *w = logical_w;
    }
    if (h) {
        *h = logical_h;
    }
}

bool SDL_GetRenderLogicalPresentation(SDL_Renderer *renderer, int *w, int *h, sdl3::SDL_RendererLogicalPresentation *mode)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW);

    /* Set the stored values of logical size and presentation mode */
    if (w) {
        *w = logical_w;
    }
    if (h) {
        *h = logical_h;
    }
    if (mode) {
        *mode = logical_mode;
    }

    return true;
}

int SDL_RenderSetViewport(SDL_Renderer * renderer, const sdl2::SDL_Rect * rect)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with new size: %d x %d", __func__, rect?rect->w:0, rect?rect->h:0);

    return ORIG_SDL2_CALL(SDL_RenderSetViewport, (renderer, rect));
}

void SDL_RenderGetViewport(SDL_Renderer * renderer, sdl2::SDL_Rect * rect)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW | LCF_TODO);
    return ORIG_SDL2_CALL(SDL_RenderGetViewport, (renderer, rect));
}

int SDL_RenderSetScale(SDL_Renderer * renderer, float scaleX, float scaleY)
{
    LOGTRACE(LCF_SDL | LCF_WINDOW | LCF_TODO, "%s called with new scaleX: %f and scaleY: %f", __func__, scaleX, scaleY);
    return ORIG_SDL2_CALL(SDL_RenderSetScale, (renderer, scaleX, scaleY));
}

void SDL_RenderGetScale(SDL_Renderer * renderer, float *scaleX, float *scaleY)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_WINDOW | LCF_TODO);
    return ORIG_SDL2_CALL(SDL_RenderGetScale, (renderer, scaleX, scaleY));
}

}