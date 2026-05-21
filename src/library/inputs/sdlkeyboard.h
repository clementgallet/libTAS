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

#ifndef LIBTAS_SDLKEYBOARD_H_INCL
#define LIBTAS_SDLKEYBOARD_H_INCL

#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include "hook.h"

namespace libtas {

/* Keyboard functions */

/**
 * Return whether a keyboard is currently connected.
 *
 * \returns true if a keyboard is connected, false otherwise.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyboards
 */
OVERRIDE bool SDL_HasKeyboard(void);

/**
 * Get a list of currently connected keyboards.
 *
 * Note that this will include any device or virtual driver that includes
 * keyboard functionality, including some mice, KVM switches, motherboard
 * power buttons, etc. You should wait for input from a device before you
 * consider it actively in use.
 *
 * \param count a pointer filled in with the number of keyboards returned, may
 *              be NULL.
 * \returns a 0 terminated array of keyboards instance IDs or NULL on failure;
 *          call SDL_GetError() for more information. This should be freed
 *          with SDL_free() when it is no longer needed.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyboardNameForID
 * \sa SDL_HasKeyboard
 */
OVERRIDE sdl3::SDL_KeyboardID * SDL_GetKeyboards(int *count);


OVERRIDE const Uint8* SDL_GetKeyboardState(int* numkeys); // SDL 2
OVERRIDE Uint8* SDL_GetKeyState(int* numkeys); // SDL 1



OVERRIDE SDL_Window* SDL_GetKeyboardFocus(void);

OVERRIDE sdl2::SDL_Keymod SDL_GetModState(void);
OVERRIDE void SDL_SetModState(sdl2::SDL_Keymod modstate);

}

#endif
