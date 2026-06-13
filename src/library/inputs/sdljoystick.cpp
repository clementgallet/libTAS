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

#include "sdljoystick.h"
#include "inputs.h"

#include "logging.h"
#include "sdl/sdlversion.h"
#include "sdl/sdldynapi.h"
#include "sdl/SDLEventQueue.h"
#include "global.h"
#include "../shared/SharedConfig.h"
#include "../shared/inputs/AllInputs.h"

#include <stdlib.h>

namespace libtas {

static int joyids[AllInputs::MAXJOYS+1] = {-1, -1, -1, -1};
static int refids[AllInputs::MAXJOYS+1] = {0, 0, 0, 0}; // joystick open/close is ref-counted
static const char* joypaths[AllInputs::MAXJOYS+1] = {"/dev/input/js0", "/dev/input/js1", "/dev/input/js2", "/dev/input/js3"};

/* Helper functions */
static bool isIdValid(SDL_Joystick* joy)
{
    if (joy == NULL)
        return false;
    int *joyid = reinterpret_cast<int*>(joy);
    if ((*joyid < 0) || (*joyid >= AllInputs::MAXJOYS) || (*joyid >= Global::shared_config.nb_controllers))
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

void mySDL_JoystickDetached(int index)
{
    if (index < 0 || index >= AllInputs::MAXJOYS || index >= Global::shared_config.nb_controllers)
        return;

    if (joyids[index] == -1)
        return;

    /* Decrease the ref count */
    refids[index]--;

    /* If no more ref, close the joystick */
    if (refids[index] == 0)
        joyids[index] = -1;
}

bool mySDL_JoystickReportEvents(int index)
{
    if (index < 0 || index >= AllInputs::MAXJOYS || index >= Global::shared_config.nb_controllers)
        return false;
    if (joyids[index] == -1)
        return false;

    const int joyevents1[] = {
        sdl1::SDL_JOYAXISMOTION,
        sdl1::SDL_JOYBUTTONDOWN,
        sdl1::SDL_JOYBUTTONUP,
        sdl1::SDL_JOYHATMOTION,
        sdl1::SDL_JOYBALLMOTION
    };

    const int joyevents2[] = {
        sdl2::SDL_JOYAXISMOTION,
        sdl2::SDL_JOYBUTTONDOWN,
        sdl2::SDL_JOYBUTTONUP,
        sdl2::SDL_JOYHATMOTION,
        sdl2::SDL_JOYBALLMOTION,
        sdl2::SDL_JOYDEVICEADDED,
        sdl2::SDL_JOYDEVICEREMOVED
    };

    bool enabled = false;
    int SDLver = get_sdlversion();

    if (SDLver == 1)
        for (int e=0; e<5; e++)
            enabled = enabled || sdlEventQueue.isEnabled(joyevents1[e]);
    if (SDLver == 2)
        for (int e=0; e<7; e++)
            enabled = enabled || sdlEventQueue.isEnabled(joyevents2[e]);

    return enabled;
}

/* Override */ int SDL_NumJoysticks(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    /* For now, we declare one joystick */
    return Global::shared_config.nb_controllers;
}

/* Override */ bool SDL_HasJoystick(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    return Global::shared_config.nb_controllers > 0;
}

sdl3::SDL_JoystickID * SDL_GetJoysticks(int *count)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (count)
        *count = Global::shared_config.nb_controllers;

    sdl3::SDL_JoystickID *ids = static_cast<sdl3::SDL_JoystickID*>(ORIG_SDL3_CALL(SDL_malloc, ((1+Global::shared_config.nb_controllers) * sizeof(sdl3::SDL_JoystickID))));
    for (int i = 0; i < Global::shared_config.nb_controllers; i++)
        ids[i] = i;
    ids[Global::shared_config.nb_controllers] = 0; // Null-terminate the array
    return ids;
}

const char* joyname = "Microsoft X-Box 360 pad";

/* Override */ const char *SDL_JoystickNameForIndex(int device_index)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (device_index >= 0 && device_index < AllInputs::MAXJOYS && device_index < Global::shared_config.nb_controllers)
        return joyname;
    return NULL;
}

/* Override */ const char * SDL_GetJoystickNameForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickNameForIndex")));

/* Override */ const char* SDL_JoystickName(SDL_Joystick* joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    /* Do not use joystick argument unless you know what you are doing.
     * Because SDL 1.2 can call this function, but the argument is
     * of type int.
     */
    return joyname;
}

/* Override */ const char * SDL_GetJoystickName(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickName")));

/* Override */ const char *SDL_JoystickPathForIndex(int device_index)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if ((device_index < 0) || (device_index >= Global::shared_config.nb_controllers))
        return nullptr;

    return joypaths[device_index];
}

/* Override */ const char * SDL_GetJoystickPathForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickPathForIndex")));

/* Override */ const char *SDL_JoystickPath(SDL_Joystick *joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValidOpen(joystick))
        return nullptr;

    int device_index = *reinterpret_cast<int*>(joystick);
    return joypaths[device_index];
}

/* Override */ SDL_Joystick *SDL_JoystickOpen(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= AllInputs::MAXJOYS))
        return NULL;
    if (device_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Opening the joystick device */
    joyids[device_index] = device_index;

    /* Increment ref count */
    refids[device_index]++;

    return reinterpret_cast<SDL_Joystick*>(&joyids[device_index]);
}

/* Override */ SDL_Joystick * SDL_OpenJoystick(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickOpen")));

/* Override */ SDL_Joystick *SDL_JoystickFromInstanceID(sdl2::SDL_JoystickID joyid)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy id %d", __func__, joyid);

    if ((joyid < 0) || (joyid >= AllInputs::MAXJOYS))
        return NULL;
    if (joyid >= Global::shared_config.nb_controllers)
        return NULL;
    if (joyids[joyid] == -1)
        /* Device not opened */
        return NULL;

    return reinterpret_cast<SDL_Joystick*>(&joyids[joyid]);
}

/* Override */ SDL_Joystick * SDL_GetJoystickFromID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickFromInstanceID")));

/* Override */ Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= AllInputs::MAXJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x045e; // vendor of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickVendorForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickGetDeviceVendor")));

/* Override */ Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= AllInputs::MAXJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x028e; // product of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickProductForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickGetDeviceProduct")));

/* Override */ Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= AllInputs::MAXJOYS))
        return 0;
    if (device_index >= Global::shared_config.nb_controllers)
        return 0;

    return 0x0114; // product version of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickProductVersionForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickGetDeviceProductVersion")));

/* Override */ sdl2::SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if ((device_index < 0) || (device_index >= AllInputs::MAXJOYS))
        return sdl2::SDL_JOYSTICK_TYPE_UNKNOWN;
    if (device_index >= Global::shared_config.nb_controllers)
        return sdl2::SDL_JOYSTICK_TYPE_UNKNOWN;

    return sdl2::SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

/* Override */ sdl2::SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if (device_index < 0 || device_index >= AllInputs::MAXJOYS || device_index >= Global::shared_config.nb_controllers)
        return -1;

    /* This function can be called without the joystick been opened... */
    return static_cast<sdl2::SDL_JoystickID>(device_index);
}

/* Xbox 360 GUID */
sdl2::SDL_JoystickGUID xinputGUID = {{0x03,0x00,0x00,0x00,0x5e,0x04,0x00,0x00,0x8e,0x02,0x00,0x00,0x14,0x01,0x00,0x00}};
sdl2::SDL_JoystickGUID nullGUID   = {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};

/* Override */ sdl2::SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
	if (device_index < 0 || device_index >= AllInputs::MAXJOYS || device_index >= Global::shared_config.nb_controllers)
	    return nullGUID;

    return xinputGUID;
}

/* Override */ SDL_GUID SDL_GetJoystickGUIDForID(sdl3::SDL_JoystickID instance_id) __attribute__((alias("SDL_JoystickGetDeviceGUID")));

/* Override */ Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x045e; // vendor of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickVendor(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetVendor")));

/* Override */ Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x028e; // product of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickProduct(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetProduct")));

/* Override */ Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    if (!isIdValid(joystick))
        return 0;

    return 0x0114; // product version of the wired xbox360 controller
}

/* Override */ Uint16 SDL_GetJoystickProductVersion(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetProductVersion")));

/* Override */ sdl2::SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);

    static_assert(static_cast<int>(sdl2::SDL_JOYSTICK_TYPE_UNKNOWN) == static_cast<int>(sdl3::SDL_JOYSTICK_TYPE_UNKNOWN), "This code assumes that sdl2 and sdl3 joystick types are equal");
    static_assert(static_cast<int>(sdl2::SDL_JOYSTICK_TYPE_GAMECONTROLLER) == static_cast<int>(sdl3::SDL_JOYSTICK_TYPE_GAMEPAD), "This code assumes that sdl2 and sdl3 joystick types are equal");

    if (!isIdValid(joystick))
        return sdl2::SDL_JOYSTICK_TYPE_UNKNOWN;

    return sdl2::SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

/* Override */ sdl3::SDL_JoystickType SDL_GetJoystickType(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetType")));

/* Override */ sdl2::SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return nullGUID;

    return xinputGUID;
}

/* Override */ SDL_GUID SDL_GetJoystickGUID(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetGUID")));

/* Override */ SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ bool SDL_JoystickConnected(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickGetAttached")));

/* Override */ int SDL_JoystickOpened(int device_index)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, device_index);
    if (!isIdValidOpen(reinterpret_cast<SDL_Joystick*>(&device_index)))
        return 0;

    return 1;
}

/* Override */ sdl2::SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValid(joystick))
        return -1;

    /* This function can be called without the joystick been opened... */
    return static_cast<sdl2::SDL_JoystickID>(*reinterpret_cast<int*>(joystick));
}

/* Override */ sdl3::SDL_JoystickID SDL_GetJoystickID(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickInstanceID")));

/* Override */ int SDL_JoystickIndex(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return -1;

    return *reinterpret_cast<int*>(joystick);
}

/* Override */ int SDL_JoystickNumAxes(SDL_Joystick* joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 6;
}

/* Override */ int SDL_GetNumJoystickAxes(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickNumAxes")));

/* Override */ int SDL_JoystickNumBalls(SDL_Joystick* joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 0;
}

/* Override */ int SDL_GetNumJoystickBalls(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickNumBalls")));

/* Override */ int SDL_JoystickNumButtons(SDL_Joystick* joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 11;
}

/* Override */ int SDL_GetNumJoystickButtons(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickNumButtons")));

/* Override */ int SDL_JoystickNumHats(SDL_Joystick* joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (!isIdValid(joystick))
        return 0;
    return 1;
}

/* Override */ int SDL_GetNumJoystickHats(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickNumHats")));

/* Override */ void SDL_JoystickUpdate(void)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
}

/* Override */  void SDL_UpdateJoysticks(void) __attribute__((alias("SDL_JoystickUpdate")));

/* Override */ int SDL_JoystickEventState(int state)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with state %d", __func__, state);
    const int joyevents1[] = {
        sdl1::SDL_JOYAXISMOTION,
        sdl1::SDL_JOYBUTTONDOWN,
        sdl1::SDL_JOYBUTTONUP,
        sdl1::SDL_JOYHATMOTION,
        sdl1::SDL_JOYBALLMOTION
    };

    const int joyevents2[] = {
        sdl2::SDL_JOYAXISMOTION,
        sdl2::SDL_JOYBUTTONDOWN,
        sdl2::SDL_JOYBUTTONUP,
        sdl2::SDL_JOYHATMOTION,
        sdl2::SDL_JOYBALLMOTION,
        sdl2::SDL_JOYDEVICEADDED,
        sdl2::SDL_JOYDEVICEREMOVED
    };

    bool enabled = false;
    int SDLver = get_sdlversion();

    switch (state) {
        case SDL_ENABLE:
            if (SDLver == 1)
                for (int e=0; e<static_cast<int>(sizeof(joyevents1)/sizeof(joyevents1[0])); e++)
                    sdlEventQueue.enable(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<static_cast<int>(sizeof(joyevents2)/sizeof(joyevents2[0])); e++)
                    sdlEventQueue.enable(joyevents2[e]);
            return 1;
        case SDL_IGNORE:
            if (SDLver == 1)
                for (int e=0; e<static_cast<int>(sizeof(joyevents1)/sizeof(joyevents1[0])); e++)
                    sdlEventQueue.disable(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<static_cast<int>(sizeof(joyevents2)/sizeof(joyevents2[0])); e++)
                    sdlEventQueue.disable(joyevents2[e]);
            return 0;
        case SDL_QUERY:
            if (SDLver == 1)
                for (int e=0; e<static_cast<int>(sizeof(joyevents1)/sizeof(joyevents1[0])); e++)
                    enabled = enabled || sdlEventQueue.isEnabled(joyevents1[e]);
            if (SDLver == 2)
                for (int e=0; e<static_cast<int>(sizeof(joyevents2)/sizeof(joyevents2[0])); e++)
                    enabled = enabled || sdlEventQueue.isEnabled(joyevents2[e]);
            if (enabled)
                return SDL_ENABLE;
            return SDL_IGNORE;
        default:
            return state;
    }
}

static const int joyevents3[] = {
    sdl3::SDL_EVENT_JOYSTICK_AXIS_MOTION,
    sdl3::SDL_EVENT_JOYSTICK_BALL_MOTION,
    sdl3::SDL_EVENT_JOYSTICK_HAT_MOTION,
    sdl3::SDL_EVENT_JOYSTICK_BUTTON_DOWN,
    sdl3::SDL_EVENT_JOYSTICK_BUTTON_UP,
    sdl3::SDL_EVENT_JOYSTICK_ADDED,
    sdl3::SDL_EVENT_JOYSTICK_REMOVED,
    sdl3::SDL_EVENT_JOYSTICK_BATTERY_UPDATED,
};

/* Override */ void SDL_SetJoystickEventsEnabled(bool enabled)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with enabled %d", __func__, enabled);

    for (int e=0; e<static_cast<int>(sizeof(joyevents3)/sizeof(joyevents3[0])); e++) {
        if (enabled)
            sdlEventQueue.enable(joyevents3[e]);
        else
            sdlEventQueue.disable(joyevents3[e]);
    }
}

/* Override */ bool SDL_JoystickEventsEnabled(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call", __func__);

    bool enabled = false;
    for (int e=0; e<static_cast<int>(sizeof(joyevents3)/sizeof(joyevents3[0])); e++)
        enabled = enabled || sdlEventQueue.isEnabled(joyevents3[e]);

    return enabled;
}

/* Override */ Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with axis %d", __func__, axis);

    if (!isIdValidOpen(joystick))
        return 0;

    if (axis >= SingleInput::AXIS_LAST)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    return Inputs::game_ai.controllers[*joyid].axes[axis];
}

/* Override */ Sint16 SDL_GetJoystickAxis(SDL_Joystick *joystick, int axis) __attribute__((alias("SDL_JoystickGetAxis")));

/* Override */ bool SDL_GetJoystickAxisInitialState(SDL_Joystick *joystick, int axis, Sint16 *state)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with axis %d", __func__, axis);

    if (!isIdValidOpen(joystick))
        return false;

    if (axis >= SingleInput::AXIS_LAST)
        return false;

    switch(axis) {
        case SingleInput::AXIS_LEFTX:
        case SingleInput::AXIS_LEFTY:
        case SingleInput::AXIS_RIGHTX:
        case SingleInput::AXIS_RIGHTY:
            return 0;
        case SingleInput::AXIS_TRIGGERLEFT:
        case SingleInput::AXIS_TRIGGERRIGHT:
            return std::numeric_limits<Sint16>::min();
    }

    return 0;
}

/* Override */ Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with hat %d", __func__, hat);

    if (!isIdValidOpen(joystick))
        return 0;

    if (hat > 0)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    Uint8 hatState = sdl2::SDL_HAT_CENTERED;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << sdl2::SDL_CONTROLLER_BUTTON_DPAD_UP))
        hatState |= sdl2::SDL_HAT_UP;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << sdl2::SDL_CONTROLLER_BUTTON_DPAD_DOWN))
        hatState |= sdl2::SDL_HAT_DOWN;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << sdl2::SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        hatState |= sdl2::SDL_HAT_LEFT;
    if (Inputs::game_ai.controllers[*joyid].buttons & (1 << sdl2::SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        hatState |= sdl2::SDL_HAT_RIGHT;

    return hatState;
}

/* Override */ Uint8 SDL_GetJoystickHat(SDL_Joystick *joystick, int hat) __attribute__((alias("SDL_JoystickGetHat")));

/* Override */ int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with ball %d", __func__, ball);
    return 0;
}

/* Override */ bool SDL_GetJoystickBall(SDL_Joystick *joystick, int ball, int *dx, int *dy)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with ball %d", __func__, ball);
    return false;
}

/* Override */ Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with button %d", __func__, button);

    if (!isIdValidOpen(joystick))
        return 0;

    if (button >= 11)
        return 0;

    int *joyid = reinterpret_cast<int*>(joystick);

    return (Inputs::game_ai.controllers[*joyid].buttons >> button) & 0x1;
}

/* Override */ bool SDL_GetJoystickButton(SDL_Joystick *joystick, int button)
{
    return SDL_JoystickGetButton(joystick, button) != 0;
}

/* Override */ int SDL_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ bool SDLCALL SDL_RumbleJoystick(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return false;
}

/* Override */ int SDL_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ bool SDL_RumbleJoystickTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return false;
}

/* Override */ SDL_bool SDL_JoystickHasLED(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_JoystickHasRumble(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_JoystickHasRumbleTriggers(SDL_Joystick *joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ bool SDL_SetJoystickLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return false;
}

/* Override */ int SDL_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return -1;
}

/* Override */ bool SDL_SendJoystickEffect(SDL_Joystick *joystick, const void *data, int size) 
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    return false;
}

/* Override */ void SDL_JoystickClose(SDL_Joystick * joystick)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK, "%s call with joy %d", __func__, joystick?*reinterpret_cast<int*>(joystick):-1);
    if (!isIdValidOpen(joystick))
        return;

    int *joyid = reinterpret_cast<int*>(joystick);

    /* Decrease the ref count */
    refids[*joyid]--;

    /* If no more ref, close the joystick */
    if (refids[*joyid] == 0)
        joyids[*joyid] = -1;
}

/* Override */ void SDL_CloseJoystick(SDL_Joystick *joystick) __attribute__((alias("SDL_JoystickClose")));

/* Override */ sdl2::SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
	return sdl2::SDL_JOYSTICK_POWER_WIRED;
}

/* Override */ sdl3::SDL_PowerState SDL_GetJoystickPowerInfo(SDL_Joystick *joystick, int *percent)
{
    LOGTRACE_SIMPLE(LCF_SDL | LCF_JOYSTICK);
    if (percent)
        *percent = -1;

    return sdl3::SDL_POWERSTATE_NO_BATTERY;
}

}
