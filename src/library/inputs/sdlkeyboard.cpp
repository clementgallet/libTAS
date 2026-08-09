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

#include "sdlkeyboard.h"
#include "inputs.h"
#include "keyboard_helper.h"

#include "logging.h"
#include "sdl/sdlwindows.h" // sdl::gameSDLWindow
#include "sdl/sdldynapi.h"
#include "../external/SDL1.h"

namespace libtas {

static bool SDL3_keyboard[sdl3::SDL_NUM_SCANCODES] = {0};
static Uint8 SDL2_keyboard[sdl2::SDL_NUM_SCANCODES] = {0};
static Uint8 SDL1_keyboard[sdl1::SDLK_LAST] = {0};

/* Override */ bool SDL_HasKeyboard(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);
    return true;
}

/* Override */ sdl3::SDL_KeyboardID * SDL_GetKeyboards(int *count)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);

    if (count)
        *count = 1;

    sdl3::SDL_KeyboardID *ids = static_cast<sdl3::SDL_KeyboardID*>(ORIG_SDL3_CALL(SDL_malloc, (sizeof(sdl3::SDL_KeyboardID))));
    ids[0] = 1; // We only have one keyboard, and its ID is 1
    ids[1] = 0; // Null-terminate the array

    return ids;
}

/* Override */ const void* SDL_GetKeyboardState( int* numkeys)
{
    // invoke_sdl2_or_sdl3_from_storage does not work in this case, so it is done manually
    if (get_sdlversion() == 3) {
        return static_cast<const void*>(sdl3::SDL_GetKeyboardState(numkeys));
    } else {
        return static_cast<const void*>(sdl2::SDL_GetKeyboardState(numkeys));
    }
}

const Uint8* sdl2::SDL_GetKeyboardState( int* numkeys)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);

    if (numkeys)
        *numkeys = sdl2::SDL_NUM_SCANCODES;

    xkeyboardToSDL2keyboard(Inputs::game_ai.keyboard, SDL2_keyboard);
    return SDL2_keyboard;
}

const bool* sdl3::SDL_GetKeyboardState( int* numkeys)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);

    if (numkeys)
        *numkeys = sdl3::SDL_NUM_SCANCODES;

    xkeyboardToSDL3keyboard(Inputs::game_ai.keyboard, SDL3_keyboard);
    return SDL3_keyboard;
}

/* Override */ Uint8* SDL_GetKeyState( int* numkeys)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);

    if (numkeys)
        *numkeys = sdl1::SDLK_LAST;

    xkeyboardToSDL1keyboard(Inputs::game_ai.keyboard, SDL1_keyboard);
    return SDL1_keyboard;
}

/* Override */ SDL_Window* SDL_GetKeyboardFocus(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);
    return sdl::gameSDLWindow;
}

/* Override */ int SDL_GetModState(void)
{
    if (get_sdlversion() == 3) {
        return sdl3::SDL_GetModState();
    } else {
        return sdl2::SDL_GetModState();
    }
}

sdl2::SDL_Keymod sdl2::SDL_GetModState(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);
    return xkeyboardToSDL2Mod(Inputs::game_ai.keyboard);
}

sdl3::SDL_Keymod sdl3::SDL_GetModState(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD);
    return xkeyboardToSDL3Mod(Inputs::game_ai.keyboard);
}

/* Override */ void SDL_SetModState(sdl2::SDL_Keymod modstate)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_KEYBOARD | LCF_TODO);
}

}
