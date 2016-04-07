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

const char* joyname = "Microsoft X-Box 360 pad";

/* Override */ const char *SDL_JoystickNameForIndex(int device_index)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (device_index < tasflags.numControllers)
        return joyname;
    return NULL;
}

/* Override */ const char* SDL_JoystickName(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    return "Microsoft X-Box 360 pad";
}

#define MAX_SDLJOYS 4
static SDL_Joystick joyids[MAX_SDLJOYS] = {-1, -1, -1, -1};

/* Helper functions */
static bool isIdValid(SDL_Joystick* joy)
{
    if (joy == NULL)
        return false;
    if ((*joy < 0) || (*joy >= MAX_SDLJOYS) || (*joy >= tasflags.numControllers))
        return false;
    return true;
}

static bool isIdValidOpen(SDL_Joystick* joy)
{
    if (!isIdValid(joy))
        return false;
    if (joyids[*joy] == -1)
        return false;
    return true;
}


/* Override */ SDL_Joystick *SDL_JoystickOpen(int device_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with joy ", device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return NULL;
    if (device_index >= tasflags.numControllers)
        return NULL;
    if (joyids[device_index] != -1)
        /* Device already opened */
        return NULL;

    /* Opening the joystick device */
    joyids[device_index] = device_index;
    return &joyids[device_index];
}

/* Override */ SDL_Joystick *SDL_JoystickFromInstanceID(SDL_JoystickID joyid)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with joy id ", joyid);

    if ((joyid < 0) || (joyid >= MAX_SDLJOYS))
        return NULL;
    if (joyid >= tasflags.numControllers)
        return NULL;
    if (joyids[joyid] == -1)
        /* Device not opened */
        return NULL;
    
    return &joyids[joyid];
}

/* Xbox 360 GUID */
SDL_JoystickGUID xinputGUID = {{3,0,0,0,94,4,0,0,142,2,0,0,20,1,0,0}};
SDL_JoystickGUID nullGUID   = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

/* Override */ SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with device ", device_index);
    if (device_index >= tasflags.numControllers)
	    return nullGUID;

    return xinputGUID;
}

/* Override */ SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return nullGUID;

    return xinputGUID;
}

/* Override */ SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    if (!isIdValidOpen(joystick))
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
    if (!isIdValidOpen(joystick))
        return -1;

    return (SDL_JoystickID) joyids[*joystick];

}

/* Override */ int SDL_JoystickNumAxes(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 6;
}

/* Override */ int SDL_JoystickNumBalls(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 0;
}

/* Override */ int SDL_JoystickNumButtons(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 11;
}

/* Override */ int SDL_JoystickNumHats(SDL_Joystick* joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 1;
}

/* Override */ void SDL_JoystickUpdate(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK | LCF_TODO);
}

/* Override */ int SDL_JoystickEventState(int state)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK | LCF_TODO);
    return 0;
}

/* Override */ Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with axis ", axis);

    if (!isIdValidOpen(joystick))
        return 0;

    if (axis >= 6)
        return 0;

    return ai.controller_axes[*joystick][axis];
}

/* Override */ Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with hat ", hat);

    if (!isIdValidOpen(joystick))
        return 0;

    if (hat > 0)
        return 0;

    Uint8 hatState = SDL_HAT_CENTERED;
    if (ai.controller_buttons[*joystick] & (1 << SDL_CONTROLLER_BUTTON_DPAD_UP))
        hatState |= SDL_HAT_UP;
    if (ai.controller_buttons[*joystick] & (1 << SDL_CONTROLLER_BUTTON_DPAD_DOWN))
        hatState |= SDL_HAT_DOWN;
    if (ai.controller_buttons[*joystick] & (1 << SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        hatState |= SDL_HAT_LEFT;
    if (ai.controller_buttons[*joystick] & (1 << SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        hatState |= SDL_HAT_RIGHT;

    return hatState;
}

/* Override */ int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with ball ", ball);
    return 0;
}

/* Override */ Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with button ", button);

    if (!isIdValidOpen(joystick))
        return 0;

    if (button >= 11)
        return 0;

    return (ai.controller_buttons[*joystick] >> button) & 0x1;
}

/* Override */ void SDL_JoystickClose(SDL_Joystick * joystick)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with joy ", *joystick);
    if (!isIdValidOpen(joystick))
        return;

    joyids[*joystick] = -1;
}

/* Override */ SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK | LCF_TODO);
	return SDL_JOYSTICK_POWER_WIRED;
}

