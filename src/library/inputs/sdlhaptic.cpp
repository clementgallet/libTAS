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

#include "sdlhaptic.h"
#include "sdl/sdldynapi.h"

#include "logging.h"

namespace libtas {

int SDL_NumHaptics(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
	return 0;
}

sdl3::SDL_HapticID * SDL_GetHaptics(int *count)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (count) {
        *count = 0;
    }

    /* The caller is responsible for freeing the returned array */
    sdl3::SDL_HapticID *haptics = (sdl3::SDL_HapticID *)ORIG_SDL3_CALL(SDL_malloc, (sizeof(sdl3::SDL_HapticID)));
    haptics[0] = 0;
    return haptics;
}

SDL_Haptic * SDL_HapticOpen(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
	return NULL;
}

SDL_Haptic * SDL_OpenHaptic(sdl3::SDL_HapticID instance_id)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    return NULL;
}

int SDL_JoystickIsHaptic(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
	return 0;
}

bool SDL_IsJoystickHaptic(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    return false;
}

SDL_Haptic *SDL_HapticOpenFromJoystick(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
	return NULL;
}

SDL_Haptic * SDL_OpenHapticFromJoystick(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    return NULL;
}

bool SDL_IsMouseHaptic(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    return false;
}

SDL_Haptic * SDL_OpenHapticFromMouse(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    return NULL;
}

void SDL_HapticClose(SDL_Haptic * haptic)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
}

void SDL_CloseHaptic(SDL_Haptic *haptic)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
}

}
