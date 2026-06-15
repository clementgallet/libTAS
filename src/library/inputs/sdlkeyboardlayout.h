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

#ifndef LIBTAS_SDLKEYBOARDLAYOUT_H_INCL
#define LIBTAS_SDLKEYBOARDLAYOUT_H_INCL

#include "hook.h"
#include "../external/SDL1.h"
#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include <cstdint>

namespace libtas {

sdl2::SDL_Scancode GetScanFromKey(sdl2::SDL_Keycode keycode);
unsigned char GetScanFromKey1(sdl1::SDLKey key);

OVERRIDE int SDL_GetKeyFromScancode(std::uintptr_t p1, std::uintptr_t p2, std::uintptr_t p3);

/**
 *  \brief Get the key code corresponding to the given scancode according
 *         to the current keyboard layout.
 *
 *  See ::SDL_Keycode for details.
 *
 *  \sa SDL_GetKeyName()
 */
sdl2::SDL_Keycode sdl2::SDL_GetKeyFromScancode(SDL_Scancode scancode);

/**
 * Get the key code corresponding to the given scancode according to the
 * current keyboard layout.
 *
 * If you want to get the keycode as it would be delivered in key events,
 * including options specified in SDL_HINT_KEYCODE_OPTIONS, then you should
 * pass `key_event` as true. Otherwise this function simply translates the
 * scancode based on the given modifier state.
 *
 * \param scancode the desired SDL_Scancode to query.
 * \param modstate the modifier state to use when translating the scancode to
 *                 a keycode.
 * \param key_event true if the keycode will be used in key events.
 * \returns the SDL_Keycode that corresponds to the given SDL_Scancode.
 *
 * \threadsafety This function is not thread safe.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyName
 * \sa SDL_GetScancodeFromKey
 */
sdl3::SDL_Keycode sdl3::SDL_GetKeyFromScancode(sdl3::SDL_Scancode scancode, sdl3::SDL_Keymod modstate, bool key_event);

OVERRIDE int SDL_GetScancodeFromKey(std::uintptr_t p1, std::uintptr_t p2);

/**
 *  \brief Get the scancode corresponding to the given key code according to the
 *         current keyboard layout.
 *
 *  See ::SDL_Scancode for details.
 *
 *  \sa SDL_GetScancodeName()
 */
sdl2::SDL_Scancode sdl2::SDL_GetScancodeFromKey(sdl2::SDL_Keycode key);

/**
 * Get the scancode corresponding to the given key code according to the
 * current keyboard layout.
 *
 * Note that there may be multiple scancode+modifier states that can generate
 * this keycode, this will just return the first one found.
 *
 * \param key the desired SDL_Keycode to query.
 * \param modstate a pointer to the modifier state that would be used when the
 *                 scancode generates this key, may be NULL.
 * \returns the SDL_Scancode that corresponds to the given SDL_Keycode.
 *
 * \threadsafety This function is not thread safe.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyFromScancode
 * \sa SDL_GetScancodeName
 */
sdl3::SDL_Scancode sdl3::SDL_GetScancodeFromKey(sdl3::SDL_Keycode key, sdl3::SDL_Keymod *modstate);

}

#endif
