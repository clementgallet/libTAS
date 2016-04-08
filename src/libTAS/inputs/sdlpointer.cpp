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
#include "../../shared/tasflags.h"
#include "../windows.h" // gameWindow
#include <X11/X.h>

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
        oldx = ai.pointer_x;
        oldy = ai.pointer_y;
        first = false;
    }

    if (x != NULL)
        *x = ai.pointer_x - oldx;
    if (y != NULL)
        *y = ai.pointer_y - oldy;

    /* Updating the old pointer coordinates */
    oldx = ai.pointer_x;
    oldy = ai.pointer_y;

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
    /* We should not support that I guess */
}

int SDL_WarpMouseGlobal(int x, int y)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call to pos (",x,",",y,")");
    /* We should not support that I guess */
    return -1;
}

void SDL_WarpMouse(Uint16 x, Uint16 y)
{
    debuglog(LCF_SDL | LCF_MOUSE, __func__, " call to pos (",x,",",y,")");
    game_ai.pointer_x = x;
    game_ai.pointer_y = y;
    /* FIXME: We need to generate a MOUSEMOTION event here! */
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



