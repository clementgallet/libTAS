/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../logging.h"
#include "../hook.h"
#include "../sdl/SDLEventQueue.h"
#include "../../shared/AllInputs.h"
#include <cstring>
#include "../GlobalState.h"
#include "../global.h"

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
        GlobalNoLog gnl;
        if (SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&index)))
        SDL_GameControllerClose(reinterpret_cast<SDL_GameController*>(&index));
        
        if (SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&index)))
        SDL_JoystickClose(reinterpret_cast<SDL_Joystick*>(&index));
    }
}

/* Override */ SDL_bool SDL_IsGameController(int joystick_index)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    if (joystick_index >= 0 && joystick_index < Global::shared_config.nb_controllers)
        return SDL_TRUE;
    return SDL_FALSE;

}

/* Override */ SDL_GameController *SDL_GameControllerOpen(int joystick_index)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    if (joystick_index < 0 || joystick_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Can't open detached game controller */
    if (!attached[joystick_index])
        return NULL;

    /* Save the opening of the game controller */
    gcids[joystick_index] = joystick_index;

    /* Increase the ref count */
    refids[joystick_index]++;

    return reinterpret_cast<SDL_GameController*>(&gcids[joystick_index]);
}

/* Override */ const char *SDL_GameControllerNameForIndex(int joystick_index)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    return joy_name;
}

/* Override */ SDL_GameControllerType SDL_GameControllerTypeForIndex(int joystick_index)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    return SDL_CONTROLLER_TYPE_XBOX360;
}

/* Override */ const char *SDL_GameControllerName(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return NULL;

    return joy_name;
}

/* Override */ SDL_GameControllerType SDL_GameControllerGetType(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return SDL_CONTROLLER_TYPE_UNKNOWN;

    return SDL_CONTROLLER_TYPE_XBOX360;    
}

/* Override */ SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController* gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    /* We simply return the same id */
    return reinterpret_cast<SDL_Joystick*>(gamecontroller);
}

/* Override */ SDL_GameController* SDL_GameControllerFromInstanceID(SDL_JoystickID joy)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joy);
    if (joy < 0 || joy >= Global::shared_config.nb_controllers)
        return NULL;
    if (gcids[joy] != -1)
        return NULL;
    return reinterpret_cast<SDL_GameController*>(&gcids[joy]);
}

const char* xbox360Mapping = "00000000000000000000000000000000,XInput Controller,a:b0,b:b1,back:b6,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,";

/* Override */ char *SDL_GameControllerMappingForGUID( SDL_JoystickGUID guid )
{
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    /* Mhhh, let's bet that the game will only call this on a guid that
     * we returned, so we return here the mapping of a xbox 360 controller.
     *
     * The game is supposed to free this, so we must allocate it.
     */
    int mapsize = strlen(xbox360Mapping);
    char* mapping = static_cast<char*>(malloc(mapsize+1));
    strcpy(mapping, xbox360Mapping);
    return mapping;
}

/* Override */ char *SDL_GameControllerMapping( SDL_GameController * gamecontroller )
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

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
    int mapsize = strlen(xbox360Mapping);
    char* mapping = static_cast<char*>(malloc(mapsize+1));
    strcpy(mapping, xbox360Mapping);
    return mapping;
}

/* Override */ char *SDL_GameControllerMappingForDeviceIndex(int joystick_index)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);

    if (joystick_index < 0 || joystick_index >= Global::shared_config.nb_controllers)
        return NULL;

    /* Return the mapping of my own xbox 360 controller.
     * The game is supposed to free the char*, so we must
     * allocate it. */
    int mapsize = strlen(xbox360Mapping);
    char* mapping = static_cast<char*>(malloc(mapsize+1));
    strcpy(mapping, xbox360Mapping);
    return mapping;
}

/* Override */ SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

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
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with state %d", __func__, state);
    const int gcevents[] = {
        SDL_CONTROLLERDEVICEADDED,
        SDL_CONTROLLERDEVICEREMOVED,
        SDL_CONTROLLERDEVICEREMAPPED,
        SDL_CONTROLLERAXISMOTION,
        SDL_CONTROLLERBUTTONDOWN,
        SDL_CONTROLLERBUTTONUP
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
    DEBUGLOGCALL(LCF_SDL | LCF_JOYSTICK);
    GlobalNoLog tnl;
    SDL_JoystickUpdate();
}

/* Override */ SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(SDL_GameController *gamecontroller,
                                 SDL_GameControllerAxis axis)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    SDL_GameControllerButtonBind b;
    b.bindType = SDL_CONTROLLER_BINDTYPE_NONE;

    if (!gamecontroller)
        return b;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return b;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return b;

    /* Check if axis is mapped */
    if ((axis < 0) || (axis >= SingleInput::AXIS_LAST))
        return b;
    
    /* Using my default xbox360 mapping */
    static int bindA[] = {0, 1, 3, 4, 2, 5};

    b.bindType = SDL_CONTROLLER_BINDTYPE_AXIS;
    b.value.axis = bindA[axis];
    
    return b;
}

/* Override */ SDL_bool SDL_GameControllerHasAxis(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    if (!gamecontroller)
        return SDL_FALSE;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return SDL_FALSE;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    /* Check if axis is mapped */
    if ((axis < 0) || (axis >= SingleInput::AXIS_LAST))
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                          SDL_GameControllerAxis axis)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and axis %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, axis);

    if (!gamecontroller)
        return 0;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if axis is valid */
    if ((axis < 0) || (axis >= SingleInput::AXIS_LAST))
        return 0;

    /* Return axis value */
    return game_ai.controller_axes[*gcid][axis];

}

/* Override */ SDL_GameControllerButtonBind 
SDL_GameControllerGetBindForButton(SDL_GameController *gamecontroller,
                                   SDL_GameControllerButton button)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    SDL_GameControllerButtonBind b;
    b.bindType = SDL_CONTROLLER_BINDTYPE_NONE;

    if (!gamecontroller)
        return b;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return b;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return b;

    /* Check if button is mapped */
    if ((button < 0) || (button >= SingleInput::BUTTON_LAST))
        return b;
    
    /* Using my default xbox360 mapping */
    static const int bindB[11] = {0, 1, 2, 3, 6, 8, 7, 9, 10, 4, 5};
    static const int bindH[4] = {1, 4, 8, 2};

    if (button < SingleInput::BUTTON_DPAD_UP) {
        b.bindType = SDL_CONTROLLER_BINDTYPE_BUTTON;
        b.value.button = bindB[button];
    }
    else {
        b.bindType = SDL_CONTROLLER_BINDTYPE_HAT;
        b.value.hat.hat = 0;
        b.value.hat.hat_mask = bindH[button-SingleInput::BUTTON_DPAD_UP];
    }
    return b;
}

/* Override */ SDL_bool SDL_GameControllerHasButton(SDL_GameController *gamecontroller,
                                                             SDL_GameControllerButton button)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    if (!gamecontroller)
        return SDL_FALSE;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return SDL_FALSE;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    /* Check if button is mapped */
    if ((button < 0) || (button >= SingleInput::BUTTON_LAST))
        return SDL_FALSE;

    return SDL_TRUE;
}


/* Override */ Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d and button %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, button);

    if (!gamecontroller)
        return 0;

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= Global::shared_config.nb_controllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if button is valid */
    if ((button < 0) || (button >= SingleInput::BUTTON_LAST))
        return 0;

    /* Return button value */
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "  return %d", (game_ai.controller_buttons[*gcid] >> button) & 0x1);

    return (game_ai.controller_buttons[*gcid] >> button) & 0x1;

}

/* Override */ SDL_bool SDL_GameControllerHasSensor(SDL_GameController *gamecontroller, SDL_SensorType type)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_GameControllerSetSensorEnabled(SDL_GameController *gamecontroller, SDL_SensorType type, SDL_bool enabled)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ SDL_bool SDL_GameControllerIsSensorEnabled(SDL_GameController *gamecontroller, SDL_SensorType type)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ float SDL_GameControllerGetSensorDataRate(SDL_GameController *gamecontroller, SDL_SensorType type)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return 0.0f;
}

/* Override */ int SDL_GameControllerGetSensorData(SDL_GameController *gamecontroller, SDL_SensorType type, float *data, int num_values)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerRumble(SDL_GameController *gamecontroller, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerRumbleTriggers(SDL_GameController *gamecontroller, Uint16 left_rumble, Uint16 right_rumble, Uint32 duration_ms)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ SDL_bool SDL_GameControllerHasLED(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_GameControllerHasRumble(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ SDL_bool SDL_GameControllerHasRumbleTriggers(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return SDL_FALSE;
}

/* Override */ int SDL_GameControllerSetLED(SDL_GameController *gamecontroller, Uint8 red, Uint8 green, Uint8 blue)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ int SDL_GameControllerSendEffect(SDL_GameController *gamecontroller, const void *data, int size)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);
    return -1;
}

/* Override */ void SDL_GameControllerClose(SDL_GameController *gamecontroller)
{
    debuglogstdio(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

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
