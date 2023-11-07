/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLKEYBOARDLAYOUT_H_INCL
#define LIBTAS_SDLKEYBOARDLAYOUT_H_INCL

#include "hook.h"
#include "../external/SDL1.h"

#include <SDL2/SDL.h>

namespace libtas {

SDL_Scancode GetScanFromKey(SDL_Keycode keycode);
unsigned char GetScanFromKey1(SDL1::SDLKey key);

/**
 *  \brief Get the key code corresponding to the given scancode according
 *         to the current keyboard layout.
 *
 *  See ::SDL_Keycode for details.
 *
 *  \sa SDL_GetKeyName()
 */
OVERRIDE SDL_Keycode SDL_GetKeyFromScancode(SDL_Scancode scancode);

/**
 *  \brief Get the scancode corresponding to the given key code according to the
 *         current keyboard layout.
 *
 *  See ::SDL_Scancode for details.
 *
 *  \sa SDL_GetScancodeName()
 */
OVERRIDE SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key);

}

#endif
