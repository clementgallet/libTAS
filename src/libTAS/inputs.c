#include "inputs.h"

struct AllInputs ai;
struct AllInputs old_ai;

Uint8 SDL_keyboard[SDL_NUM_SCANCODES] = {0};

SDL_JoystickID joyid[4] = {-1, -1, -1, -1};
const char joy_name[] = "Universal joystick";

/* Do we have to generate controller events? */
int controller_events = 1;

/* Override */ Uint8* SDL_GetKeyboardState( int* numkeys)
{
    (void) numkeys; // Remove unused warning
    debuglog(LCF_SDL | LCF_KEYBOARD, "%s call.", __func__);
    xkeyboardToSDLkeyboard(ai.keyboard, SDL_keyboard);
    //*numkeys = 512;
    return SDL_keyboard;
}

/* Generate released keyboard input events */
int generateKeyUpEvent(SDL_Event *event, void* gameWindow)
{
    int i, j;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (old_ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (old_ai.keyboard[i] == ai.keyboard[j]) {
                /* Key was not released */
                break;
            }
        }
        if (j == 16) {
            /* Key was released. Generate event */
            event->type = SDL_KEYUP;
            event->key.state = SDL_RELEASED;
            event->key.windowID = SDL_GetWindowID_real(gameWindow);
            event->key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, old_ai.keyboard[i]);
            event->key.keysym = keysym;

            /* Update old keyboard state so that this event won't trigger inifinitely */
            old_ai.keyboard[i] = XK_VoidSymbol;
            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key %d.", event->key.keysym.sym);
            return 1;

        }
    }
    return 0;
}

/* Generate pressed keyboard input events */
int generateKeyDownEvent(SDL_Event *event, void* gameWindow)
{
    int i,j,k;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (ai.keyboard[i] == old_ai.keyboard[j]) {
                /* Key was not pressed */
                break;
            }
        }
        if (j == 16) {
            /* Key was pressed. Generate event */
            event->type = SDL_KEYDOWN;
            event->key.state = SDL_PRESSED;
            event->key.windowID = SDL_GetWindowID_real(gameWindow);
            event->key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, ai.keyboard[i]);
            event->key.keysym = keysym;

            /* Update old keyboard state so that this event won't trigger inifinitely */
            for (k=0; k<16; k++)
                if (old_ai.keyboard[k] == XK_VoidSymbol) {
                    /* We found an empty space to put our key*/
                    old_ai.keyboard[k] = ai.keyboard[i];
                    break;
                }

            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key %d.", event->key.keysym.sym);
            return 1;

        }
    }
    return 0;
}

int generateControllerEvent(SDL_Event* event)
{
    if (!controller_events)
        return 0;
    int ji = 0;
    for (ji=0; ji<4; ji++) if (joyid[ji] != -1) {
        /* We have a joystick detected */

        /* Check for axes change */
        int axis;
        for (axis=0; axis<6; axis++) {
            if (ai.controller_axes[ji][axis] != old_ai.controller_axes[ji][axis]) {
                /* We got a change in a controller axis value */

                /* Fill the event structure */
                event->type = SDL_CONTROLLERAXISMOTION;
                event->caxis.timestamp = SDL_GetTicks_real(); // TODO: maybe our timer instead
                event->caxis.which = joyid[ji];
                event->caxis.axis = axis;
                event->caxis.value = ai.controller_axes[ji][axis];

                /* Upload the old AllInput struct */
                old_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];

                /* Return the event */
                return 1;
            }
        }

        /* Check for button change */
        int bi;
        unsigned short buttons = ai.controller_buttons[ji];
        unsigned short old_buttons = old_ai.controller_buttons[ji];

        for (bi=0; bi<16; bi++) {
            if (((buttons >> bi) & 0x1) != ((old_buttons >> bi) & 0x1)) {
                /* We got a change in a button state */

                /* Fill the event structure */
                if ((buttons >> bi) & 0x1) {
                    event->type = SDL_CONTROLLERBUTTONDOWN;
                    event->cbutton.state = SDL_PRESSED;
                }
                else {
                    event->type = SDL_CONTROLLERBUTTONUP;
                    event->cbutton.state = SDL_RELEASED;
                }
                event->cbutton.timestamp = SDL_GetTicks_real(); // TODO: maybe our timer instead
                event->cbutton.which = joyid[ji];
                event->cbutton.button = bi;

                /* Upload the old AllInput struct */
                old_ai.controller_buttons[ji] ^= (1 << bi);

                /* Return the event */
                return 1;
            }
        }
    }

    /* No controller event generated */
    return 0;

}

/**
 *  Count the number of joysticks attached to the system right now
 */
/* Override */ int SDL_NumJoysticks(void)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call", __func__);
    /* For now, we declare one joystick */
    return 1;
}

/**
 *  Is the joystick on this index supported by the game controller interface?
 */
/* Override */ SDL_bool SDL_IsGameController(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    /* For now, enabling 1 game controller */
    if (joystick_index == 0)
        return SDL_TRUE;
    return SDL_FALSE;

}


/**
 *  Open a game controller for use.
 *  The index passed as an argument refers to the N'th game controller on the system.
 *  This index is the value which will identify this controller in future controller
 *  events.
 *
 *  \return A controller identifier, or NULL if an error occurred.
 */
/* Override */ SDL_GameController *SDL_GameControllerOpen(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    SDL_GameController* gc_id = malloc(1);
    *gc_id = joystick_index;

    /* Save the opening of the game controller */
    joyid[joystick_index] = joystick_index;
    return gc_id;
}

/**
 *  Get the implementation dependent name of a game controller.
 *  This can be called before any controllers are opened.
 *  If no name can be found, this function returns NULL.
 */
/* Override */ const char *SDL_GameControllerNameForIndex(int joystick_index)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, joystick_index);
    return joy_name;
}

/**
 *  Return the name for this currently opened controller
 */
/* Override */ const char *SDL_GameControllerName(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call with id %d", __func__, *gamecontroller);
    return joy_name;
}

/**
 *  Returns SDL_TRUE if the controller has been opened and currently connected,
 *  or SDL_FALSE if it has not.
 */
/* Override */ SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK | LCF_FRAME, "%s call with id %d", __func__, *gamecontroller);
    if (joyid[*gamecontroller] != -1)
        return SDL_TRUE;
    return SDL_FALSE;
}

/**
 *  Enable/disable controller event polling.
 *
 *  If controller events are disabled, you must call SDL_GameControllerUpdate()
 *  yourself and check the state of the controller when you want controller
 *  information.
 *
 *  The state can be one of ::SDL_QUERY, ::SDL_ENABLE or ::SDL_IGNORE.
 */
/* Override */ int SDL_GameControllerEventState(int state)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call with state %d", __func__, state);
    switch (state) {
        case 1:
            controller_events = 1;
            return 1;
        case 0:
            controller_events = 0;
            return 0;
        case -1:
            return controller_events;
        default:
            return state;
    }
}

/**
 *  Get the current state of an axis control on a game controller.
 *
 *  The state is a value ranging from -32768 to 32767.
 *
 *  The axis indices start at index 0.
 */
/* Override */ Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                                SDL_GameControllerAxis axis)
{
    debuglog(LCF_SDL | LCF_JOYSTICK | LCF_FRAME, "%s call with id %d and axis %d", __func__, *gamecontroller, axis);

    /* Check if controller is available */
    if (joyid[*gamecontroller] == -1)
        return 0;

    /* Check if axis is valid */
    if ((axis < 0) || (axis >= SDL_CONTROLLER_AXIS_MAX ))
        return 0;

    /* Return axis value */
    return ai.controller_axes[joyid[*gamecontroller]][axis];

}

/**
 *  Get the current state of a button on a game controller.
 *
 *  The button indices start at index 0.
 */
/* Override */ Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button)
{
    debuglog(LCF_SDL | LCF_JOYSTICK | LCF_FRAME, "%s call with id %d and button %d", __func__, *gamecontroller, button);

    /* Check if controller is available */
    if (joyid[*gamecontroller] == -1)
        return 0;

    /* Check if button is valid */
    if ((button < 0) || (button >= SDL_CONTROLLER_BUTTON_MAX ))
        return 0;

    /* Return button value */
    return (ai.controller_buttons[joyid[*gamecontroller]] >> button) & 0x1;

}

/**
 *  Close a controller previously opened with SDL_GameControllerOpen().
 */
/* Override */ void SDL_GameControllerClose(SDL_GameController *gamecontroller)
{
    debuglog(LCF_SDL | LCF_JOYSTICK | LCF_FRAME, "%s call with id %d", __func__, *gamecontroller);

    joyid[*gamecontroller] = -1;

}




