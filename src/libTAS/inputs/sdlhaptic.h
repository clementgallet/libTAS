/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLHAPTIC_H_INCL
#define LIBTAS_SDLHAPTIC_H_INCL

#include <SDL2/SDL.h>
#include "../global.h"
#include "sdljoystick.h"

namespace libtas {

/**
 *  \brief Count the number of haptic devices attached to the system.
 *
 *  \return Number of haptic devices detected on the system.
 */
OVERRIDE int SDL_NumHaptics(void);

/**
 *  \brief Opens a Haptic device for usage.
 *
 *  The index passed as an argument refers to the N'th Haptic device on this
 *  system.
 *
 *  When opening a haptic device, its gain will be set to maximum and
 *  autocenter will be disabled.  To modify these values use
 *  SDL_HapticSetGain() and SDL_HapticSetAutocenter().
 *
 *  \param device_index Index of the device to open.
 *  \return Device identifier or NULL on error.
 *
 *  \sa SDL_HapticIndex
 *  \sa SDL_HapticOpenFromMouse
 *  \sa SDL_HapticOpenFromJoystick
 *  \sa SDL_HapticClose
 *  \sa SDL_HapticSetGain
 *  \sa SDL_HapticSetAutocenter
 *  \sa SDL_HapticPause
 *  \sa SDL_HapticStopAll
 */
OVERRIDE SDL_Haptic * SDL_HapticOpen(int device_index);

/**
 *  \brief Checks to see if a joystick has haptic features.
 *
 *  \param joystick Joystick to test for haptic capabilities.
 *  \return 1 if the joystick is haptic, 0 if it isn't
 *          or -1 if an error ocurred.
 *
 *  \sa SDL_HapticOpenFromJoystick
 */
OVERRIDE int SDL_JoystickIsHaptic(SDL_Joystick * joystick);

/**
 *  \brief Opens a Haptic device for usage from a Joystick device.
 *
 *  You must still close the haptic device separately.  It will not be closed
 *  with the joystick.
 *
 *  When opening from a joystick you should first close the haptic device before
 *  closing the joystick device.  If not, on some implementations the haptic
 *  device will also get unallocated and you'll be unable to use force feedback
 *  on that device.
 *
 *  \param joystick Joystick to create a haptic device from.
 *  \return A valid haptic device identifier on success or NULL on error.
 *
 *  \sa SDL_HapticOpen
 *  \sa SDL_HapticClose
 */
OVERRIDE SDL_Haptic *SDL_HapticOpenFromJoystick(SDL_Joystick *joystick);

/**
 *  \brief Closes a Haptic device previously opened with SDL_HapticOpen().
 *
 *  \param haptic Haptic device to close.
 */
OVERRIDE void SDL_HapticClose(SDL_Haptic * haptic);

}

#endif
