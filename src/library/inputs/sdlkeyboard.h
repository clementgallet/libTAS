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

#include <cstdint>

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


OVERRIDE const void* SDL_GetKeyboardState(int* numkeys);

/**
 * Get a snapshot of the current state of the keyboard.
 *
 * The pointer returned is a pointer to an internal SDL array. It will be
 * valid for the whole lifetime of the application and should not be freed by
 * the caller.
 *
 * A array element with a value of 1 means that the key is pressed and a value
 * of 0 means that it is not. Indexes into this array are obtained by using
 * SDL_Scancode values.
 *
 * Use SDL_PumpEvents() to update the state array.
 *
 * This function gives you the current state after all events have been
 * processed, so if a key or button has been pressed and released before you
 * process events, then the pressed state will never show up in the
 * SDL_GetKeyboardState() calls.
 *
 * Note: This function doesn't take into account whether shift has been
 * pressed or not.
 *
 * \param numkeys if non-NULL, receives the length of the returned array.
 * \returns a pointer to an array of key states.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_PumpEvents
 * \sa SDL_ResetKeyboard
 */
const Uint8* sdl2::SDL_GetKeyboardState(int* numkeys);

/**
 * Get a snapshot of the current state of the keyboard.
 *
 * The pointer returned is a pointer to an internal SDL array. It will be
 * valid for the whole lifetime of the application and should not be freed by
 * the caller.
 *
 * A array element with a value of true means that the key is pressed and a
 * value of false means that it is not. Indexes into this array are obtained
 * by using SDL_Scancode values.
 *
 * Use SDL_PumpEvents() to update the state array.
 *
 * This function gives you the current state after all events have been
 * processed, so if a key or button has been pressed and released before you
 * process events, then the pressed state will never show up in the
 * SDL_GetKeyboardState() calls.
 *
 * Note: This function doesn't take into account whether shift has been
 * pressed or not.
 *
 * \param numkeys if non-NULL, receives the length of the returned array.
 * \returns a pointer to an array of key states.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_PumpEvents
 * \sa SDL_ResetKeyboard
 */
const bool* sdl3::SDL_GetKeyboardState(int* numkeys);

OVERRIDE Uint8* SDL_GetKeyState(int* numkeys); // SDL 1



OVERRIDE SDL_Window* SDL_GetKeyboardFocus(void);

OVERRIDE int SDL_GetModState(void);

/**
 * Get the current key modifier state for the keyboard.
 *
 * \returns an OR'd combination of the modifier keys for the keyboard. See
 *          SDL_Keymod for details.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GetKeyboardState
 * \sa SDL_SetModState
 */
sdl2::SDL_Keymod sdl2::SDL_GetModState(void);

/**
 * Get the current key modifier state for the keyboard.
 *
 * \returns an OR'd combination of the modifier keys for the keyboard.
 *
 * \threadsafety It is safe to call this function from any thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetKeyboardState
 * \sa SDL_SetModState
 */
sdl3::SDL_Keymod sdl3::SDL_GetModState(void);

OVERRIDE void SDL_SetModState(sdl2::SDL_Keymod modstate);

}

#endif
