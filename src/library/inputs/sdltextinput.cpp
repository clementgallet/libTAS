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

#include "sdltextinput.h"
#include "../../external/SDL1.h"
#include "inputs.h"
#include "keyboard_helper.h"
#include "../logging.h"
#include "../../shared/AllInputs.h"

namespace libtas {

static bool isTextInputActive = false;

void SDL_StartTextInput(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
    isTextInputActive = true;
}

SDL_bool SDL_IsTextInputActive(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
    return isTextInputActive?SDL_TRUE:SDL_FALSE;
}

void SDL_StopTextInput(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
    isTextInputActive = false;
}

void SDL_SetTextInputRect(SDL_Rect *rect)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
}

/* SDL 1 */
static bool isUnicodeEnabled = false;

int SDL_EnableUNICODE(int enable)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);

    if (enable == -1) {
        return isUnicodeEnabled?1:0;
    }
    isUnicodeEnabled = enable;
    return enable;
}

}
