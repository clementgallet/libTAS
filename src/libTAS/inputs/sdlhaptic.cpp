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

#include "sdlhaptic.h"
#include "../logging.h"

int SDL_NumHaptics(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
	return 0;
}

SDL_Haptic * SDL_HapticOpen(int device_index)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
	return NULL;
}

int SDL_JoystickIsHaptic(SDL_Joystick * joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
	return 0;
}

SDL_Haptic *SDL_HapticOpenFromJoystick(SDL_Joystick *joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
	return NULL;
}

void SDL_HapticClose(SDL_Haptic * haptic)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
}

