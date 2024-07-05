/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <SDL2/SDL.h>

#include "hook.h"

namespace libtas {

/**
 *  \brief Start accepting Unicode text input events.
 *         This function will show the on-screen keyboard if supported.
 *
 *  \sa SDL_StopTextInput()
 *  \sa SDL_SetTextInputRect()
 *  \sa SDL_HasScreenKeyboardSupport()
 */
OVERRIDE void SDL_StartTextInput(void);

/**
 *  \brief Return whether or not Unicode text input events are enabled.
 *
 *  \sa SDL_StartTextInput()
 *  \sa SDL_StopTextInput()
 */
OVERRIDE SDL_bool SDL_IsTextInputActive(void);

/**
 *  \brief Stop receiving any text input events.
 *         This function will hide the on-screen keyboard if supported.
 *
 *  \sa SDL_StartTextInput()
 *  \sa SDL_HasScreenKeyboardSupport()
 */
OVERRIDE void SDL_StopTextInput(void);

/**
 *  \brief Set the rectangle used to type Unicode text inputs.
 *         This is used as a hint for IME and on-screen keyboard placement.
 *
 *  \sa SDL_StartTextInput()
 */
OVERRIDE void SDL_SetTextInputRect(SDL_Rect *rect);

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
