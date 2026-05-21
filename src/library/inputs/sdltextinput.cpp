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

#include "sdltextinput.h"

#include "logging.h"
#include "../external/SDL1.h"
#include "../external/SDL2.h"
#include "../external/SDL3.h"

namespace libtas {

static bool isTextInputActive = true;

/* Override */ bool SDL_StartTextInput(SDL_Window *window)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
    isTextInputActive = true;
    return true;
}

SDL_bool SDL_IsTextInputActive(void)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
    return isTextInputActive?SDL_TRUE:SDL_FALSE;
}

bool SDL_TextInputActive(SDL_Window *window)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
    return isTextInputActive;
}

bool SDL_StopTextInput(SDL_Window *window)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
    isTextInputActive = false;
    return true;
}

void SDL_SetTextInputRect(const sdl2::SDL_Rect *rect)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
}

bool SDL_SetTextInputArea(SDL_Window *window, const sdl3::SDL_Rect *rect, int cursor)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);
    return true;
}

/* SDL 1 */
static bool isUnicodeEnabled = false;

int SDL_EnableUNICODE(int enable)
{
    LOGTRACE(LCF_SDL | LCF_KEYBOARD);

    if (enable == -1) {
        return isUnicodeEnabled?1:0;
    }
    isUnicodeEnabled = enable;
    return enable;
}

}
