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

#include "sdljoystick.h"
#include "inputs.h"
#include "../logging.h"
#include "../hook.h"
#include "../../shared/AllInputs.h"
#include "../../shared/tasflags.h"
#include <stdlib.h>

/* Override */ int SDL_NumJoysticks(void)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call.");
    /* For now, we declare one joystick */
    return tasflags.numControllers;
}

/* Xbox 360 GUID */
SDL_JoystickGUID xinputGUID = {{3,0,0,0,94,4,0,0,142,2,0,0,20,1,0,0}};

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
	return xinputGUID;
}

const char* SDL_JoystickName(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return "Microsoft X-Box 360 pad";
}

int SDL_JoystickNumAxes(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return 6;
}

int SDL_JoystickNumBalls(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return 0;
}

int SDL_JoystickNumButtons(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return 11;
}

int SDL_JoystickNumHats(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return 1;
}

