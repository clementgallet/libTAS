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

#include "sdlkeyboard.h"
#include "../../external/SDL1.h"
#include "inputs.h"
#include "keyboard_helper.h"
#include "../logging.h"
#include "../../shared/AllInputs.h"
#include "../sdl/sdlwindows.h" // sdl::gameSDLWindow

namespace libtas {

static Uint8 SDL_keyboard[SDL_NUM_SCANCODES] = {0};
static Uint8 SDL1_keyboard[SDL1::SDLK_LAST] = {0};

/* Override */ const Uint8* SDL_GetKeyboardState( int* numkeys)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);

    if (numkeys)
        *numkeys = SDL_NUM_SCANCODES;

    xkeyboardToSDLkeyboard(game_ai.keyboard, SDL_keyboard);
    return SDL_keyboard;
}

/* Override */ Uint8* SDL_GetKeyState( int* numkeys)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);

    if (numkeys)
        *numkeys = SDL1::SDLK_LAST;

    xkeyboardToSDL1keyboard(game_ai.keyboard, SDL1_keyboard);
    return SDL1_keyboard;
}

/* Override */ SDL_Window* SDL_GetKeyboardFocus(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
    return sdl::gameSDLWindow;
}

/* Override */ SDL_Keymod SDL_GetModState(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD);
    return xkeyboardToSDLMod(game_ai.keyboard);
}

/* Override */ void SDL_SetModState(SDL_Keymod modstate)
{
    DEBUGLOGCALL(LCF_SDL | LCF_KEYBOARD | LCF_TODO);
}

}
