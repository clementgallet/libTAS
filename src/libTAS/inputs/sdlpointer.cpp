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

#include "sdlpointer.h"
#include "inputs.h"
#include "../logging.h"
#include "../hook.h"
#include "../../shared/AllInputs.h"
#include "../sdlwindows.h" // gameWindow
#include "../DeterministicTimer.h" // detTimer
#include <X11/X.h>
#include "../EventQueue.h"

SDL_Window *SDL_GetMouseFocus(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_MOUSE);
    return gameWindow;
}

Uint32 SDL_GetMouseState(int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_MOUSE);
    if (x != NULL)
        *x = ai.pointer_x;
    if (y != NULL)
        *y = ai.pointer_y;

    /* Translating Xlib pointer mask to SDL pointer state */
    Uint32 sdlmask = 0;
    if (ai.pointer_mask & Button1Mask)
        sdlmask |= SDL_BUTTON_LMASK;
    if (ai.pointer_mask & Button2Mask)
        sdlmask |= SDL_BUTTON_MMASK;
    if (ai.pointer_mask & Button3Mask)
        sdlmask |= SDL_BUTTON_RMASK;
    if (ai.pointer_mask & Button4Mask)
        sdlmask |= SDL_BUTTON_X1MASK;
    if (ai.pointer_mask & Button5Mask)
        sdlmask |= SDL_BUTTON_X2MASK;
    return sdlmask;
}

Uint32 SDL_GetGlobalMouseState(int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_MOUSE);
    /* We don't support global mouse state. We consider that the window
     * is located at the bottom left of the screen and just output the
     * same result as SDL_GetMouseState().
     * Hopefully games won't use this function anyway.
     */
    return SDL_GetMouseState(x, y);
}

Uint32 SDL_GetRelativeMouseState(int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_MOUSE);

    static bool first = true;
    static int oldx = 0;
    static int oldy = 0;

    /* For the first call, just output zero deltas */
    if (first) {
        oldx = game_ai.pointer_x;
        oldy = game_ai.pointer_y;
        first = false;
    }

    if (x != NULL)
        *x = game_ai.pointer_x - oldx;
    if (y != NULL)
        *y = game_ai.pointer_y - oldy;

    /* Updating the old pointer coordinates */
    oldx = game_ai.pointer_x;
    oldy = game_ai.pointer_y;

    /* Translating Xlib pointer mask to SDL pointer state */
    Uint32 sdlmask = 0;
    if (ai.pointer_mask & Button1Mask)
        sdlmask |= SDL_BUTTON_LMASK;
    if (ai.pointer_mask & Button2Mask)
        sdlmask |= SDL_BUTTON_MMASK;
    if (ai.pointer_mask & Button3Mask)
        sdlmask |= SDL_BUTTON_RMASK;
    if (ai.pointer_mask & Button4Mask)
        sdlmask |= SDL_BUTTON_X1MASK;
    if (ai.pointer_mask & Button5Mask)
        sdlmask |= SDL_BUTTON_X2MASK;
    return sdlmask;

}

void SDL_WarpMouseInWindow(SDL_Window * window, int x, int y)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call to pos (",x,",",y,")");

    /* We have to generate an MOUSEMOTION event. */
    SDL_Event event2;
    event2.type = SDL_MOUSEMOTION;
    struct timespec time = detTimer.getTicks();
    event2.motion.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
    event2.motion.windowID = orig::SDL_GetWindowID(gameWindow);
    event2.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

    /* Build up mouse state */
    event2.motion.state = 0;
    if (ai.pointer_mask & Button1Mask)
        event2.motion.state |= SDL_BUTTON_LMASK;
    if (ai.pointer_mask & Button2Mask)
        event2.motion.state |= SDL_BUTTON_MMASK;
    if (ai.pointer_mask & Button3Mask)
        event2.motion.state |= SDL_BUTTON_RMASK;
    if (ai.pointer_mask & Button4Mask)
        event2.motion.state |= SDL_BUTTON_X1MASK;
    if (ai.pointer_mask & Button5Mask)
        event2.motion.state |= SDL_BUTTON_X2MASK;

    event2.motion.x = x;
    event2.motion.y = y;
    event2.motion.xrel = game_ai.pointer_x - x;
    event2.motion.yrel = game_ai.pointer_y - y;
    sdlEventQueue.insert(&event2);

    /* Update the pointer coordinates */
    game_ai.pointer_x = x;
    game_ai.pointer_y = y;
}

int SDL_WarpMouseGlobal(int x, int y)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call to pos (",x,",",y,")");

    /* Should we support this? */
    SDL_WarpMouseInWindow(nullptr, x, y);
    return 0;
}

void SDL_WarpMouse(Uint16 x, Uint16 y)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call to pos (",x,",",y,")");

    /* We have to generate an MOUSEMOTION event. */
    SDL1::SDL_Event event1;
    event1.type = SDL1::SDL_MOUSEMOTION;
    event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

    /* Build up mouse state */
    event1.motion.state = 0;
    if (ai.pointer_mask & Button1Mask)
        event1.motion.state |= SDL1::SDL_BUTTON_LMASK;
    if (ai.pointer_mask & Button2Mask)
        event1.motion.state |= SDL1::SDL_BUTTON_MMASK;
    if (ai.pointer_mask & Button3Mask)
        event1.motion.state |= SDL1::SDL_BUTTON_RMASK;
    if (ai.pointer_mask & Button4Mask)
        event1.motion.state |= SDL1::SDL_BUTTON_X1MASK;
    if (ai.pointer_mask & Button5Mask)
        event1.motion.state |= SDL1::SDL_BUTTON_X2MASK;

    event1.motion.x = x;
    event1.motion.y = y;
    event1.motion.xrel = (Sint16)(game_ai.pointer_x - x);
    event1.motion.yrel = (Sint16)(game_ai.pointer_y - y);
    sdlEventQueue.insert(&event1);

    /* Update the pointer coordinates */
    game_ai.pointer_x = x;
    game_ai.pointer_y = y;
}

SDL_bool relativeMode = SDL_FALSE;

int SDL_SetRelativeMouseMode(SDL_bool enabled)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call with ", enabled);
    relativeMode = enabled;
    return 0;
}

int SDL_CaptureMouse(SDL_bool enabled)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call with ", enabled);
    /* We should disable capture anyway */
    return 0;
}

SDL_bool SDL_GetRelativeMouseMode(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_MOUSE);
    return relativeMode;
}


int SDL_ShowCursor(int toggle)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call with ", toggle);

    /* We keep the state of the cursor, but we keep it shown. */
    static int showCursor = 1;
    if (toggle != -1)
        showCursor = toggle;
    return showCursor;
}
