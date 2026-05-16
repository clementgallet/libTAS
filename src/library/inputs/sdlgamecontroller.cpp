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

#include "sdlgamecontroller.h"
#include "sdljoystick.h"
#include "inputs.h"

#include "logging.h"
#include "hook.h"
#include "sdl/SDLEventQueue.h"
#include "GlobalState.h"
#include "global.h"

#include <cstring>

namespace libtas {

static int gcids[4] = {-1, -1, -1, -1};
static int refids[4] = {0, 0, 0, 0}; // GC open/close is ref-counted
static const char joy_name[] = "XInput Controller";

/* We support game controllers being disconnected during gameplay */
static bool attached[4] = {true, true, true, true}; // all controllers are attached
                                                    // at startup

bool mySDL_GameControllerIsAttached(int index)
{
    return attached[index];
}

void mySDL_GameControllerChangeAttached(int index)
{
    attached[index] = !attached[index];

    if (!attached[index]) {
        /* Disconnect connected joystick */

        if (gcids[index] != -1) {
            /* Decrease the ref count */
            refids[index]--;

            /* If no more ref, close the controller */
            if (refids[index] == 0)
                gcids[index] = -1;
        }

        mySDL_JoystickDetached(index);
    }
}

bool mySDL_GameControllerReportEvents(int index)
{
    if (index < 0 || index >= Global::shared_config.nb_controllers)
        return false;
    if (gcids[index] == -1)
        return false;

    const int gcevents[] = {
        sdl2::SDL_CONTROLLERDEVICEADDED,
        sdl2::SDL_CONTROLLERDEVICEREMOVED,
        sdl2::SDL_CONTROLLERDEVICEREMAPPED,
        sdl2::SDL_CONTROLLERAXISMOTION,
        sdl2::SDL_CONTROLLERBUTTONDOWN,
        sdl2::SDL_CONTROLLERBUTTONUP
    };

    bool enabled = false;

    for (int e=0; e<6; e++)
        enabled = enabled || sdlEventQueue.isEnabled(gcevents[e]);

    return enabled;
}

/* Override */ SDL_bool SDL_IsGameController(int joystick_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    if (joystick_index >= 0 && joystick_index < Global::shared_config.nb_controllers)
        return SDL_TRUE;
    return SDL_FALSE;

}

/* Override */ sdl2::SDL_GameController *SDL_GameControllerOpen(int joystick_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    if (joystick_index < 0 || joystick_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Can't open detached game controller */
    if (!attached[joystick_index])
        return NULL;

    /* Save the opening of the game controller */
    gcids[joystick_index] = joystick_index;

    /* Increase the ref count */
    refids[joystick_index]++;

    return reinterpret_cast<sdl2::SDL_GameController*>(&gcids[joystick_index]);
}

/* Override */ const char *SDL_GameControllerNameForIndex(int joystick_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    return joy_name;
}

/* Override */ sdl2::SDL_GameControllerType SDL_GameControllerTypeForIndex(int joystick_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    return sdl2::SDL_CONTROLLER_TYPE_XBOX360;
}

/* Override */ const char *SDL_GameControllerName(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return NULL;

    return joy_name;
}

/* Override */ sdl2::SDL_GameControllerType SDL_GameControllerGetType(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return sdl2::SDL_CONTROLLER_TYPE_UNKNOWN;

    return sdl2::SDL_CONTROLLER_TYPE_XBOX360;    
}

/* Override */ sdl2::SDL_Joystick* SDL_GameControllerGetJoystick(sdl2::SDL_GameController* gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    /* We simply return the same id */
    return reinterpret_cast<sdl2::SDL_Joystick*>(gamecontroller);
}

/* Override */ sdl2::SDL_GameController* SDL_GameControllerFromInstanceID(sdl2::SDL_JoystickID joy)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joy);
    if (joy < 0 || joy >= Global::shared_config.nb_controllers)
        return NULL;
    if (gcids[joy] == -1)
        return NULL;
    return reinterpret_cast<sdl2::SDL_GameController*>(&gcids[joy]);
}

const char* xbox360Mapping = "00000000000000000000000000000000,XInput Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,";

static char* duplicateMapping()
{
    size_t mapsize = std::strlen(xbox360Mapping) + 1;
    char* mapping = static_cast<char*>(std::malloc(mapsize));
    if (mapping == nullptr)
        return nullptr;

    std::memcpy(mapping, xbox360Mapping, mapsize);
    return mapping;
}

/* Override */ char *SDL_GameControllerMappingForGUID( sdl2::SDL_JoystickGUID guid )
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    /* Mhhh, let's bet that the game will only call this on a guid that
     * we returned, so we return here the mapping of a xbox 360 controller.
     *
     * The game is supposed to free this, so we must allocate it.
     */
    return duplicateMapping();
}

/* Override */ char *SDL_GameControllerMapping( sdl2::SDL_GameController * gamecontroller )
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return NULL;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return NULL;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return NULL;

    /* Return the mapping of my own xbox 360 controller.
     * The game is supposed to free the char*, so we must
     * allocate it. */
    return duplicateMapping();
}

/* Override */ char *SDL_GameControllerMappingForDeviceIndex(int joystick_index)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);

    if (joystick_index < 0 || joystick_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Return the mapping of my own xbox 360 controller.
     * The game is supposed to free the char*, so we must
     * allocate it. */
    return duplicateMapping();
}

/* Override */ SDL_bool SDL_GameControllerGetAttached(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return SDL_FALSE;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return SDL_FALSE;
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ int SDL_GameControllerEventState(int state)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with state %d", __func__, state);
    const int gcevents[] = {
        sdl2::SDL_CONTROLLERDEVICEADDED,
        sdl2::SDL_CONTROLLERDEVICEREMOVED,
        sdl2::SDL_CONTROLLERDEVICEREMAPPED,
        sdl2::SDL_CONTROLLERAXISMOTION,
        sdl2::SDL_CONTROLLERBUTTONDOWN,
        sdl2::SDL_CONTROLLERBUTTONUP
    };

    bool enabled = false;
    switch (state) {
        case SDL_ENABLE:
            for (int e=0; e<6; e++)
                sdlEventQueue.enable(gcevents[e]);
            return 1;
        case SDL_IGNORE:
            for (int e=0; e<6; e++)
                sdlEventQueue.disable(gcevents[e]);
            return 0;
        case SDL_QUERY:
            for (int e=0; e<6; e++)
                enabled = enabled || sdlEventQueue.isEnabled(gcevents[e]);
            if (enabled)
                return SDL_ENABLE;
            return SDL_IGNORE;
        default:
            return state;
    }
}

/* Override */ void SDL_GameControllerUpdate(void)
{
    LOGTRACE(LCF_SDL | LCF_JOYSTICK);
    GlobalNoLog tnl;
    SDL_JoystickUpdate();
}

/* Override */ sdl2::SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(sdl2::SDL_GameController *gamecontroller,
                                 sdl2::SDL_GameControllerAxis axis)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    sdl2::SDL_GameControllerButtonBind b;
    b.bindType = sdl2::SDL_CONTROLLER_BINDTYPE_NONE;

    if (!gamecontroller)
        return b;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return b;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return b;

    /* Check if axis is mapped */
    if ((axis < 0) || (axis >= (sdl2::SDL_GameControllerAxis)SingleInput::AXIS_LAST))
        return b;
    
    /* Using my default xbox360 mapping */
    static int bindA[] = {0, 1, 3, 4, 2, 5};

    b.bindType = sdl2::SDL_CONTROLLER_BINDTYPE_AXIS;
    b.value.axis = bindA[axis];
    
    return b;
}

/* Override */ SDL_bool SDL_GameControllerHasAxis(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_GameControllerAxis axis)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    if (!gamecontroller)
        return SDL_FALSE;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return SDL_FALSE;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    /* Check if axis is mapped */
    if ((axis < 0) || (axis >= (sdl2::SDL_GameControllerAxis) SingleInput::AXIS_LAST))
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ Sint16 SDL_GameControllerGetAxis(sdl2::SDL_GameController *gamecontroller,
                                          sdl2::SDL_GameControllerAxis axis)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    if (!gamecontroller)
        return 0;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if axis is valid */
    if ((axis < 0) || (axis >= (sdl2::SDL_GameControllerAxis) SingleInput::AXIS_LAST))
        return 0;

    /* Return axis value */
    return Inputs::game_ai.controllers[*gcid].axes[axis];

}

/* Override */ sdl2::SDL_GameControllerButtonBind 
SDL_GameControllerGetBindForButton(sdl2::SDL_GameController *gamecontroller,
                                   sdl2::SDL_GameControllerButton button)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    sdl2::SDL_GameControllerButtonBind b;
    b.bindType = sdl2::SDL_CONTROLLER_BINDTYPE_NONE;

    if (!gamecontroller)
        return b;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return b;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return b;

    /* Check if button is mapped */
    if ((button < 0) || (button >= (sdl2::SDL_GameControllerButton) SingleInput::BUTTON_LAST))
        return b;
    
    /* Using my default xbox360 mapping */
    static const int bindB[15] = {0, 1, 2, 3, 6, 8, 7, 9, 10, 4, 5, /* hats */ 1, 4, 8, 2};

    if (button < (sdl2::SDL_GameControllerButton) SingleInput::BUTTON_DPAD_UP) {
        b.bindType = sdl2::SDL_CONTROLLER_BINDTYPE_BUTTON;
        b.value.button = bindB[button];
    }
    else {
        b.bindType = sdl2::SDL_CONTROLLER_BINDTYPE_HAT;
        b.value.hat.hat = 0;
        b.value.hat.hat_mask = bindB[button];
    }
    return b;
}

/* Override */ SDL_bool SDL_GameControllerHasButton(sdl2::SDL_GameController *gamecontroller,
                                                             sdl2::SDL_GameControllerButton button)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    if (!gamecontroller)
        return SDL_FALSE;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return SDL_FALSE;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    /* Check if button is mapped */
    if ((button < 0) || (button >= (sdl2::SDL_GameControllerButton) SingleInput::BUTTON_LAST))
        return SDL_FALSE;

    return SDL_TRUE;
}


/* Override */ Uint8 SDL_GameControllerGetButton(sdl2::SDL_GameController *gamecontroller,
                                                 sdl2::SDL_GameControllerButton button)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    if (!gamecontroller)
        return 0;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if button is valid */
    if ((button < 0) || (button >= (sdl2::SDL_GameControllerButton) SingleInput::BUTTON_LAST))
        return 0;

    /* Return button value */
    LOG(LL_DEBUG, LCF_SDL | LCF_JOYSTICK, "  return %d", (Inputs::game_ai.controllers[*gcid].buttons >> button) & 0x1);

    return (Inputs::game_ai.controllers[*gcid].buttons >> button) & 0x1;

}

/* Override */ SDL_bool SDL_GameControllerHasSensor(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_SensorType type)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_GameControllerSetSensorEnabled(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_SensorType type, SDL_bool enabled)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ SDL_bool SDL_GameControllerIsSensorEnabled(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_SensorType type)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ float SDL_GameControllerGetSensorDataRate(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_SensorType type)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return 0.0f;
}

/* Override */ int SDL_GameControllerGetSensorData(sdl2::SDL_GameController *gamecontroller, sdl2::SDL_SensorType type, float *data, int num_values)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerRumble(sdl2::SDL_GameController *gamecontroller, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerRumbleTriggers(sdl2::SDL_GameController *gamecontroller, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ SDL_bool SDL_GameControllerHasLED(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_GameControllerHasRumble(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_GameControllerHasRumbleTriggers(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_GameControllerSetLED(sdl2::SDL_GameController *gamecontroller, Uint8 red, Uint8 green, Uint8 blue)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerSendEffect(sdl2::SDL_GameController *gamecontroller, const void *data, int size)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ void SDL_GameControllerClose(sdl2::SDL_GameController *gamecontroller)
{
    LOG(LL_TRACE, LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return;

    /* Decrease the ref count */
    refids[*gcid]--;

    /* If no more ref, close the controller */
    if (refids[*gcid] == 0)
        gcids[*gcid] = -1;
}

}
