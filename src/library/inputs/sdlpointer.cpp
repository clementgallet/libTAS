/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdlpointer.h"
#include "inputs.h"

#include "logging.h"
#include "hook.h"
#include "DeterministicTimer.h" // detTimer
#include "sdl/SDLEventQueue.h"
#include "sdl/sdlwindows.h" // sdl::gameSDLWindow
#include "global.h"
#include "GlobalState.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GetWindowID)
DECLARE_ORIG_POINTER(SDL_WarpMouseInWindow)
DEFINE_ORIG_POINTER(SDL_WarpMouse)

SDL_Window *SDL_GetMouseFocus(void)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    return sdl::gameSDLWindow;
}

Uint32 SDL_GetMouseState(int *x, int *y)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);

    if (x != NULL)
        *x = Inputs::game_ai.pointer.x;
    if (y != NULL)
        *y = Inputs::game_ai.pointer.y;

    /* Translating pointer mask to SDL pointer state */
    return SingleInput::toSDL2PointerMask(Inputs::game_ai.pointer.mask);
}

Uint32 SDL_GetGlobalMouseState(int *x, int *y)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* We don't support global mouse state. We consider that the window
     * is located at the bottom left of the screen and just output the
     * same result as SDL_GetMouseState().
     * Hopefully games won't use this function anyway.
     */
    return SDL_GetMouseState(x, y);
}

Uint32 SDL_GetRelativeMouseState(int *x, int *y)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);

    static bool first = true;
    static int oldx = 0;
    static int oldy = 0;

    /* For the first call, just output zero deltas */
    if (first) {
        oldx = Inputs::game_unclipped_pointer.x;
        oldy = Inputs::game_unclipped_pointer.y;
        first = false;
    }

    if (x != NULL)
        *x = Inputs::game_unclipped_pointer.x - oldx;
    if (y != NULL)
        *y = Inputs::game_unclipped_pointer.y - oldy;

    /* Updating the old pointer coordinates */
    oldx = Inputs::game_unclipped_pointer.x;
    oldy = Inputs::game_unclipped_pointer.y;

    /* Translating pointer mask to SDL pointer state */
    return SingleInput::toSDL2PointerMask(Inputs::game_ai.pointer.mask);
}

void SDL_WarpMouseInWindow(SDL_Window * window, int x, int y)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call to pos (%d,%d)", __func__, x, y);

    /* We have to generate an MOUSEMOTION event. */
    SDL_Event event2;
    event2.type = SDL_MOUSEMOTION;
    struct timespec time = DeterministicTimer::get().getTicks();
    event2.motion.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
    LINK_NAMESPACE_SDL2(SDL_GetWindowID);
    event2.motion.windowID = orig::SDL_GetWindowID(sdl::gameSDLWindow);
    event2.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

    /* Build up mouse state */
    event2.motion.state = SingleInput::toSDL2PointerMask(Inputs::game_ai.pointer.mask);
    event2.motion.x = x;
    event2.motion.y = y;
    event2.motion.xrel = Inputs::game_ai.pointer.x - x;
    event2.motion.yrel = Inputs::game_ai.pointer.y - y;
    sdlEventQueue.insert(&event2);

    /* Update the pointer coordinates */
    Inputs::game_ai.pointer.x = x;
    Inputs::game_ai.pointer.y = y;
    
    if (Global::shared_config.mouse_prevent_warp) {
        return;
    }

    /* When warping cursor, real and game cursor position are now synced */
    Inputs::old_ai.pointer.x = x;
    Inputs::old_ai.pointer.y = y;

    LINK_NAMESPACE_SDL2(SDL_WarpMouseInWindow);
    NATIVECALL(orig::SDL_WarpMouseInWindow(window, x, y));    
}

int SDL_WarpMouseGlobal(int x, int y)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call to pos (%d,%d)", __func__, x, y);

    /* Should we support this? */
    SDL_WarpMouseInWindow(nullptr, x, y);
    return 0;
}

void SDL_WarpMouse(Uint16 x, Uint16 y)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call to pos (%d,%d)", __func__, x, y);

    /* We have to generate an MOUSEMOTION event. */
    SDL1::SDL_Event event1;
    event1.type = SDL1::SDL_MOUSEMOTION;
    event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

    /* Build up mouse state */
    event1.motion.state = SingleInput::toSDL1PointerMask(Inputs::game_ai.pointer.mask);
    event1.motion.x = x;
    event1.motion.y = y;
    event1.motion.xrel = (Sint16)(Inputs::game_ai.pointer.x - x);
    event1.motion.yrel = (Sint16)(Inputs::game_ai.pointer.y - y);
    sdlEventQueue.insert(&event1);

    /* Update the pointer coordinates */
    Inputs::game_ai.pointer.x = x;
    Inputs::game_ai.pointer.y = y;
    
    if (Global::shared_config.mouse_prevent_warp) {
        return;
    }

    LINK_NAMESPACE_SDL1(SDL_WarpMouse);
    NATIVECALL(orig::SDL_WarpMouse(x, y));
}

SDL_bool relativeMode = SDL_FALSE;

int SDL_SetRelativeMouseMode(SDL_bool enabled)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call with %d", __func__, enabled);
    relativeMode = enabled;
    return 0;
}

int SDL_CaptureMouse(SDL_bool enabled)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call with %d", __func__, enabled);
    /* We should disable capture anyway */
    return 0;
}

SDL_bool SDL_GetRelativeMouseMode(void)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    return relativeMode;
}

SDL_Cursor *SDL_CreateCursor(const Uint8 * data, const Uint8 * mask, int w, int h, int hot_x, int hot_y)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* Return some non-null value */
    return reinterpret_cast<SDL_Cursor*>(1);
}

SDL_Cursor *SDL_CreateColorCursor(SDL_Surface *surface, int hot_x, int hot_y)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* Return some non-null value */
    return reinterpret_cast<SDL_Cursor*>(1);
}

SDL_Cursor *SDL_CreateSystemCursor(SDL_SystemCursor id)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* Return some non-null value */
    return reinterpret_cast<SDL_Cursor*>(1);
}

void SDL_SetCursor(SDL_Cursor * cursor)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
}

SDL_Cursor *SDL_GetCursor(void)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* Return some non-null value */
    return reinterpret_cast<SDL_Cursor*>(1);
}

SDL_Cursor *SDL_GetDefaultCursor(void)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    /* Return some non-null value */
    return reinterpret_cast<SDL_Cursor*>(1);
}

void SDL_FreeCursor(SDL_Cursor * cursor)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
}

int SDL_ShowCursor(int toggle)
{
    LOG(LL_TRACE, LCF_SDL | LCF_MOUSE, "%s call with %d", __func__, toggle);

    /* We keep the state of the cursor, but we keep it shown. */
    static int showCursor = 1;
    if (toggle != -1)
        showCursor = toggle;
    return showCursor;
}

static SDL_Window* pointer_grab_sdl_window = nullptr;
DECLARE_ORIG_POINTER(SDL_GetWindowSize)

void SDL_SetWindowGrab(SDL_Window * window, SDL_bool grabbed)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);

    if (grabbed) {
        pointer_grab_sdl_window = window;
        
        int w, h;
        LINK_NAMESPACE_SDL2(SDL_GetWindowSize);
        orig::SDL_GetWindowSize(window, &w, &h);

        Inputs::pointer_clipping = true;
        Inputs::clipping_x = 0;
        Inputs::clipping_y = 0;
        Inputs::clipping_w = w;
        Inputs::clipping_h = h;
            
        if (Inputs::game_ai.pointer.x < Inputs::clipping_x) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x);
            Inputs::game_ai.pointer.x = Inputs::clipping_x;
        }
        else if (Inputs::game_ai.pointer.x >= (Inputs::clipping_x + Inputs::clipping_w)) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x + Inputs::clipping_w - 1);
            Inputs::game_ai.pointer.x = Inputs::clipping_x + Inputs::clipping_w - 1;
        }
        
        if (Inputs::game_ai.pointer.y < Inputs::clipping_y) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y);
            Inputs::game_ai.pointer.y = Inputs::clipping_y;
        }
        else if (Inputs::game_ai.pointer.y >= (Inputs::clipping_y + Inputs::clipping_h)) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y + Inputs::clipping_h - 1);
            Inputs::game_ai.pointer.y = Inputs::clipping_y + Inputs::clipping_h - 1;
        }
    }
    else {
        pointer_grab_sdl_window = nullptr;
        Inputs::pointer_clipping = false;
    }
}

void SDL_SetWindowMouseGrab(SDL_Window * window, SDL_bool grabbed)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);

    if (grabbed) {
        pointer_grab_sdl_window = window;
        
        int w, h;
        LINK_NAMESPACE_SDL2(SDL_GetWindowSize);
        orig::SDL_GetWindowSize(window, &w, &h);

        Inputs::pointer_clipping = true;
        Inputs::clipping_x = 0;
        Inputs::clipping_y = 0;
        Inputs::clipping_w = w;
        Inputs::clipping_h = h;
            
        if (Inputs::game_ai.pointer.x < Inputs::clipping_x) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x);
            Inputs::game_ai.pointer.x = Inputs::clipping_x;
        }
        else if (Inputs::game_ai.pointer.x >= (Inputs::clipping_x + Inputs::clipping_w)) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer x from %d to %d", Inputs::game_ai.pointer.x, Inputs::clipping_x + Inputs::clipping_w - 1);
            Inputs::game_ai.pointer.x = Inputs::clipping_x + Inputs::clipping_w - 1;
        }
        
        if (Inputs::game_ai.pointer.y < Inputs::clipping_y) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y);
            Inputs::game_ai.pointer.y = Inputs::clipping_y;
        }
        else if (Inputs::game_ai.pointer.y >= (Inputs::clipping_y + Inputs::clipping_h)) {
            LOG(LL_DEBUG, LCF_SDL | LCF_MOUSE, "   warping pointer y from %d to %d", Inputs::game_ai.pointer.y, Inputs::clipping_y + Inputs::clipping_h - 1);
            Inputs::game_ai.pointer.y = Inputs::clipping_y + Inputs::clipping_h - 1;
        }
    }
    else {
        pointer_grab_sdl_window = nullptr;
        Inputs::pointer_clipping = false;
    }
}

SDL_bool SDL_GetWindowGrab(SDL_Window * window)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    return (window == pointer_grab_sdl_window) ? SDL_TRUE : SDL_FALSE;
}

SDL_bool SDL_GetWindowMouseGrab(SDL_Window * window)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    return (window == pointer_grab_sdl_window) ? SDL_TRUE : SDL_FALSE;
}

SDL_Window* SDL_GetGrabbedWindow(void)
{
    LOGTRACE(LCF_SDL | LCF_MOUSE);
    return pointer_grab_sdl_window;
}

}
