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

#ifndef LIBTAS_SDLTEXTINPUT_H_INCL
#define LIBTAS_SDLTEXTINPUT_H_INCL

#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include "hook.h"

namespace libtas {

/**
 * Start accepting Unicode text input events in a window.
 *
 * This function will enable text input (SDL_EVENT_TEXT_INPUT and
 * SDL_EVENT_TEXT_EDITING events) in the specified window. Please use this
 * function paired with SDL_StopTextInput().
 *
 * Text input events are not received by default.
 *
 * On some platforms using this function shows the screen keyboard and/or
 * activates an IME, which can prevent some key press events from being passed
 * through.
 *
 * \param window the window to enable text input.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetTextInputArea
 * \sa SDL_StartTextInputWithProperties
 * \sa SDL_StopTextInput
 * \sa SDL_TextInputActive
 */
OVERRIDE bool SDL_StartTextInput(SDL_Window *window);

/* SDL2 version that can be fitted by SDL3 function above */
// OVERRIDE void SDL_StartTextInput(void)

/**
 *  \brief Return whether or not Unicode text input events are enabled.
 *
 *  \sa SDL_StartTextInput()
 *  \sa SDL_StopTextInput()
 */
OVERRIDE SDL_bool SDL_IsTextInputActive(void);

/**
 * Check whether or not Unicode text input events are enabled for a window.
 *
 * \param window the window to check.
 * \returns true if text input events are enabled else false.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_StartTextInput
 */
OVERRIDE bool SDL_TextInputActive(SDL_Window *window);

/**
 * Stop receiving any text input events in a window.
 *
 * If SDL_StartTextInput() showed the screen keyboard, this function will hide
 * it.
 *
 * \param window the window to disable text input.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_StartTextInput
 */
OVERRIDE bool SDL_StopTextInput(SDL_Window *window);

/* SDL2 version that can be fitted by SDL3 function above */
// OVERRIDE void SDL_StopTextInput(void);

/**
 *  \brief Set the rectangle used to type Unicode text inputs.
 *         This is used as a hint for IME and on-screen keyboard placement.
 *
 *  \sa SDL_StartTextInput()
 */
OVERRIDE void SDL_SetTextInputRect(const sdl2::SDL_Rect *rect);

/**
 * Set the area used to type Unicode text input.
 *
 * Native input methods may place a window with word suggestions near the
 * cursor, without covering the text being entered.
 *
 * \param window the window for which to set the text input area.
 * \param rect the SDL_Rect representing the text input area, in window
 *             coordinates, or NULL to clear it.
 * \param cursor the offset of the current cursor location relative to
 *               `rect->x`, in window coordinates.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetTextInputArea
 * \sa SDL_StartTextInput
 */
OVERRIDE bool SDL_SetTextInputArea(SDL_Window *window, const sdl3::SDL_Rect *rect, int cursor);

/**
 * Enable/Disable UNICODE translation of keyboard input.
 *
 * This translation has some overhead, so translation defaults off.
 *
 * @param[in] enable
 * If 'enable' is 1, translation is enabled.
 * If 'enable' is 0, translation is disabled.
 * If 'enable' is -1, the translation state is not changed.
 *
 * @return It returns the previous state of keyboard translation.
 */
OVERRIDE int SDL_EnableUNICODE(int enable);


}

#endif
