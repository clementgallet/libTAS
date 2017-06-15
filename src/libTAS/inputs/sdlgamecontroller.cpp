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

#include "sdlgamecontroller.h"
#include "sdljoystick.h"
#include "inputs.h"
#include "../logging.h"
#include "../hook.h"
#include "../EventQueue.h"
#include "../../shared/AllInputs.h"
#include "../../shared/SharedConfig.h"
#include <cstring>
#include "../GlobalState.h"

namespace libtas {

static int gcids[4] = {-1, -1, -1, -1};
static const char joy_name[] = "XInput Controller";

/* Override */ SDL_bool SDL_IsGameController(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", joystick_index);
    if (joystick_index >= 0 && joystick_index < shared_config.numControllers)
        return SDL_TRUE;
    return SDL_FALSE;

}

/* Override */ SDL_GameController *SDL_GameControllerOpen(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", joystick_index);
    if (joystick_index < 0 || joystick_index >= shared_config.numControllers)
        return NULL;

    /* Save the opening of the game controller */
    gcids[joystick_index] = joystick_index;

    return reinterpret_cast<SDL_GameController*>(&gcids[joystick_index]);
}

/* Override */ const char *SDL_GameControllerNameForIndex(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", joystick_index);
    return joy_name;
}

/* Override */ const char *SDL_GameControllerName(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    if (!gamecontroller)
        return NULL;

    int* gcid = reinterpret_cast<int*>(gamecontroller);
    debuglog(LCF_SDL | LCF_JOYSTICK, "  Id ", *gcid);
    return joy_name;
}

/* Override */ SDL_Joystick* SDL_GameControllerGetJoystick(SDL_GameController* gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    /* We simply return the same id */
    return reinterpret_cast<SDL_Joystick*>(gamecontroller);
}

/* Override */ SDL_GameController* SDL_GameControllerFromInstanceID(SDL_JoystickID joy)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", joy);
    if (joy < 0 || joy >= shared_config.numControllers)
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
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= shared_config.numControllers)
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

/* Override */ SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= shared_config.numControllers)
        return SDL_FALSE;
    if (gcids[*gcid] == -1)
        return SDL_FALSE;

    return SDL_TRUE;
}

/* Override */ int SDL_GameControllerEventState(int state)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with state ", state);
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

/* Override */ Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                          SDL_GameControllerAxis axis)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, " and axis ", axis);

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= shared_config.numControllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if axis is valid */
    if ((axis < 0) || (axis >= SDL_CONTROLLER_AXIS_MAX ))
        return 0;

    /* Return axis value */
    return game_ai.controller_axes[*gcid][axis];

}

/* Override */ Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1, " and button ", button);

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= shared_config.numControllers)
        return 0;

    /* Check if controller is available */
    if (gcids[*gcid] == -1)
        return 0;

    /* Check if button is valid */
    if ((button < 0) || (button >= SDL_CONTROLLER_BUTTON_MAX ))
        return 0;

    /* Return button value */
    return (game_ai.controller_buttons[*gcid] >> button) & 0x1;

}

/* Override */ void SDL_GameControllerClose(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, __func__, " call with id ", gamecontroller?*reinterpret_cast<int*>(gamecontroller):-1);

    int* gcid = reinterpret_cast<int*>(gamecontroller);

    if (*gcid < 0 || *gcid >= shared_config.numControllers)
        return;

    gcids[*gcid] = -1;
}

}
