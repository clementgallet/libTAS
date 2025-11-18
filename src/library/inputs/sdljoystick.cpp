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

#include "sdljoystick.h"
#include "inputs.h"

#include "logging.h"
#include "sdl/sdlversion.h"
#include "sdl/SDLEventQueue.h"
#include "global.h"
#include "../shared/SharedConfig.h"

#include <stdlib.h>

namespace libtas {

#define MAX_SDLJOYS 4
static int joyids[MAX_SDLJOYS] = {-1, -1, -1, -1};
static int refids[4] = {0, 0, 0, 0}; // joystick open/close is ref-counted
static const char* joypaths[MAX_SDLJOYS] = {"/dev/input/js0", "/dev/input/js1", "/dev/input/js2", "/dev/input/js3"};

/* Helper functions */
static bool isIdValid(SDL_Joystick* joy)
{
    if (joy == NULL)
        return false;
    int *joyid = reinterpret_cast<int*>(joy);
    if ((*joyid < 0) || (*joyid >= MAX_SDLJOYS) || (*joyid >= Global::shared_config.nb_controllers))
        return false;
    return true;
}

static bool isIdValidOpen(SDL_Joystick* joy)
{
    if (!isIdValid(joy))
        return false;
    int *joyid = reinterpret_cast<int*>(joy);
    if (joyids[*joyid] == -1)
        return false;
    return true;
}

/* Override */ int SDL_NumJoysticks(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    /* For now, we declare one joystick */
    return Global::shared_config.nb_controllers;
}

const char* joyname = "Microsoft X-Box 360 pad";

/* Override */ const char *SDL_JoystickNameForIndex(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (device_index < Global::shared_config.nb_controllers)
        return joyname;
    return NULL;
}

/* Override */ const char* SDL_JoystickName(SDL_Joystick* joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    /* Do not use joystick argument unless you know what you are doing.
     * Because SDL 1.2 can call this function, but the argument is
     * of type int.
     */
    return joyname;
}

/* Override */ const char *SDL_JoystickPathForIndex(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if ((device_index < 0) || (device_index >= Global::shared_config.nb_controllers))
        return nullptr;

    return joypaths[device_index];
}

/* Override */ const char *SDL_JoystickPath(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValidOpen(joystick))
        return nullptr;

    int device_index = *reinterpret_cast<int*>(joystick);
    return joypaths[device_index];
}

/* Override */ SDL_Joystick *SDL_JoystickOpen(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return NULL;
    if (device_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Opening the joystick device */
    joyids[device_index] = device_index;

    /* Increment ref count */
    refids[device_index]++;

    return reinterpret_cast<SDL_Joystick*>(&joyids[device_index]);
}

/* Override */ SDL_Joystick *SDL_JoystickFromInstanceID(SDL_JoystickID joyid)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy id %d", __func__, joyid);

    if ((joyid < 0) || (joyid >= MAX_SDLJOYS))
        return NULL;
    if (joyid >= Global::shared_config.nb_controllers)
        return NULL;
    if (joyids[joyid] == -1)
        /* Device not opened */
        return NULL;

    return reinterpret_cast<SDL_Joystick*>(&joyids[joyid]);
}

/* Override */ Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x045e; // vendor of the wired xbox360 controller
}

/* Override */ Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x028e; // product of the wired xbox360 controller
}

/* Override */ Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x0114; // product version of the wired xbox360 controller
}

/* Override */ SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= MAX_SDLJOYS))
        return SDL_JOYSTICK_TYPE_UNKNOWN;
    if (device_index >= Global::shared_config.nb_controllers)
        return SDL_JOYSTICK_TYPE_UNKNOWN;

    return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

/* Override */ SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if (device_index < 0 || device_index >= MAX_SDLJOYS || device_index >= Global::shared_config.nb_controllers)
        return -1;

    /* This function can be called without the joystick been opened... */
    return static_cast<SDL_JoystickID>(device_index);
}

/* Xbox 360 GUID */
SDL_JoystickGUID xinputGUID = {{0x03,0x00,0x00,0x00,0x5e,0x04,0x00,0x00,0x8e,0x02,0x00,0x00,0x14,0x01,0x00,0x00}};
SDL_JoystickGUID nullGUID   = {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};

/* Override */ SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if (device_index >= Global::shared_config.nb_controllers)
	    return nullGUID;

    return xinputGUID;
}

/* Override */ Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x045e; // vendor of the wired xbox360 controller
}

/* Override */ Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x028e; // product of the wired xbox360 controller
}

/* Override */ Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x0114; // product version of the wired xbox360 controller
}

/* Override */ SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return SDL_JOYSTICK_TYPE_UNKNOWN;

    return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

/* Override */ SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return nullGUID;

    return xinputGUID;
}

/* Override */ SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ int SDL_JoystickOpened(int device_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if (!isIdValidOpen(reinterpret_cast<SDL_Joystick*>(&device_index)))
        return 0;

    return 1;
}

/* Override */ SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValid(joystick))
        return -1;

    /* This function can be called without the joystick been opened... */
    return static_cast<SDL_JoystickID>(*reinterpret_cast<int*>(joystick));
}

int SDL_JoystickIndex(SDL_Joystick *joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return -1;

    return *reinterpret_cast<int*>(joystick);
}

/* Override */ int SDL_JoystickNumAxes(SDL_Joystick* joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 6;
}

/* Override */ int SDL_JoystickNumBalls(SDL_Joystick* joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 0;
}

/* Override */ int SDL_JoystickNumButtons(SDL_Joystick* joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 11;
}

/* Override */ int SDL_JoystickNumHats(SDL_Joystick* joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 1;
}

/* Override */ void SDL_JoystickUpdate(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
}

/* Override */ int SDL_JoystickEventState(int state)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with state %d", __func__, state);
    const int joyevents1[] = {
        SDL1::SDL_JOYAXISMOTION,
        SDL1::SDL_JOYBUTTONDOWN,
        SDL1::SDL_JOYBUTTONUP,
        SDL1::SDL_JOYHATMOTION,
        SDL1::SDL_JOYBALLMOTION
    };

    const int joyevents2[] = {
        SDL_JOYAXISMOTION,
        SDL_JOYBUTTONDOWN,
        SDL_JOYBUTTONUP,
        SDL_JOYHATMOTION,
        SDL_JOYBALLMOTION,
        SDL_JOYDEVICEADDED,
        SDL_JOYDEVICEREMOVED
    };

    bool enabled = false;
    int SDLver = get_sdlversion();

    switch (state) {
        case SDL_ENABLE:
            if (SDLver == 1)
                for (int e=0; e<5; e++)
                    sdlEventQueue.enable(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<7; e++)
                    sdlEventQueue.enable(joyevents2[e]);
            return 1;
        case SDL_IGNORE:
            if (SDLver == 1)
                for (int e=0; e<5; e++)
                    sdlEventQueue.disable(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<7; e++)
                    sdlEventQueue.disable(joyevents2[e]);
            return 0;
        case SDL_QUERY:
            if (SDLver == 1)
                for (int e=0; e<5; e++)
                    enabled = enabled || sdlEventQueue.isEnabled(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<7; e++)
                    enabled = enabled || sdlEventQueue.isEnabled(joyevents2[e]);
            if (enabled)
                return SDL_ENABLE;
            return SDL_IGNORE;
        default:
            return state;
    }
}

/* Override */ Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with axis %d", __func__, axis);

    if (!isIdValidOpen(joystick))
        return 0;

    if (axis >= 6)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    return Inputs::game_ai.controllers[*joyid].axes[axis];
}

/* Override */ Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with hat %d", __func__, hat);

    if (!isIdValidOpen(joystick))
        return 0;

    if (hat > 0)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    Uint8 hatState = SDL_HAT_CENTERED;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_UP))
        hatState |= SDL_HAT_UP;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_DOWN))
        hatState |= SDL_HAT_DOWN;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        hatState |= SDL_HAT_LEFT;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        hatState |= SDL_HAT_RIGHT;

    return hatState;
}

/* Override */ int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with ball %d", __func__, ball);
    return 0;
}

/* Override */ Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with button %d", __func__, button);

    if (!isIdValidOpen(joystick))
        return 0;

    if (button >= 11)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    return (Inputs::game_ai.controllers[*joyid].buttons >> button) & 0x1;
}

/* Override */ int SDL_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ int SDL_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ SDL_bool SDL_JoystickHasLED(SDL_Joystick *joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_JoystickHasRumble(SDL_Joystick *joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_JoystickHasRumbleTriggers(SDL_Joystick *joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ int SDL_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ void SDL_JoystickClose(SDL_Joystick * joystick)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return;

    int *joyid = reinterpret_cast<int*>(joystick);

    /* Decrease the ref count */
    refids[*joyid]--;

    /* If no more ref, close the joystick */
    if (refids[*joyid] == 0)
        joyids[*joyid] = -1;
}

/* Override */ SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
	return SDL_JOYSTICK_POWER_WIRED;
}

}
