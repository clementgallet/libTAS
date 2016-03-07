#include "inputs.h"
#include "keyboard_helper.h"
#include "logging.h"
#include "hook.h"
#include "../shared/inputs.h"
#include <X11/keysym.h>
#include <stdlib.h>

struct AllInputs ai;
struct AllInputs old_ai;

Uint8 SDL_keyboard[SDL_NUM_SCANCODES] = {0};
Uint8 SDL1_keyboard[SDLK1_LAST] = {0};

SDL_JoystickID joyid[4] = {-1, -1, -1, -1};
const char joy_name[] = "Universal joystick";

/* Do we have to generate controller events? */
int controller_events = 1;

/* Override */ Uint8* SDL_GetKeyboardState( int* numkeys)
{
    debuglog(LCF_SDL | LCF_KEYBOARD | LCF_FRAME, "%s call.", __func__);

    if (numkeys)
        *numkeys = SDL_NUM_SCANCODES;

    xkeyboardToSDLkeyboard(ai.keyboard, SDL_keyboard);
    //*numkeys = 512;
    return SDL_keyboard;
}

/* Override */ Uint8* SDL_GetKeyState( int* numkeys)
{
    debuglog(LCF_SDL | LCF_KEYBOARD | LCF_FRAME, "%s call.", __func__);

    if (numkeys)
        *numkeys = SDLK1_LAST;

    xkeyboardToSDL1keyboard(ai.keyboard, SDL1_keyboard);
    return SDL1_keyboard;
}

/* Generate released keyboard input events
 * The number of returned events is at most num,
 * and we indicate if we want to update the input struct so that
 * the events won't be returned again 
 */

int generateKeyUpEvent(SDL_Event *events, void* gameWindow, int num, int update)
{
    int evi = 0;
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
            events[evi].type = SDL_KEYUP;
            events[evi].key.state = SDL_RELEASED;
            events[evi].key.windowID = SDL_GetWindowID_real(gameWindow);
            events[evi].key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, old_ai.keyboard[i]);
            events[evi].key.keysym = keysym;

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                old_ai.keyboard[i] = XK_VoidSymbol;
            }
            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key %d.", events[evi].key.keysym.sym);
            evi++;

            /* If we reached the asked number of events, returning */
            if (evi == num)
                return evi;

        }
    }
    /* We did not reached the asked number of events, returning the number */
    return evi;
}

/* TODO: Duplicate of the above code with SDL 1 instead */

int generateKeyUp1Event(SDL1_Event *events, int num, int update)
{
    int evi = 0;
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
            events[evi].type = SDL1_KEYUP;
            events[evi].key.which = 0; // FIXME: I don't know what is going here
            events[evi].key.state = SDL_RELEASED;

            SDL_keysym keysym;
            xkeysymToSDL1(&keysym, old_ai.keyboard[i]);
            events[evi].key.keysym = keysym;

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                old_ai.keyboard[i] = XK_VoidSymbol;
            }
            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key %d.", events[evi].key.keysym.sym);
            evi++;

            /* If we reached the asked number of events, returning */
            if (evi == num)
                return evi;

        }
    }
    /* We did not reached the asked number of events, returning the number */
    return evi;
}

/* Generate pressed keyboard input events */
int generateKeyDownEvent(SDL_Event *events, void* gameWindow, int num, int update)
{
    int evi = 0;
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
            events[evi].type = SDL_KEYDOWN;
            events[evi].key.state = SDL_PRESSED;
            events[evi].key.windowID = SDL_GetWindowID_real(gameWindow);
            events[evi].key.timestamp = SDL_GetTicks_real() - 1; // TODO: Should use our deterministic timer instead

            SDL_Keysym keysym;
            xkeysymToSDL(&keysym, ai.keyboard[i]);
            events[evi].key.keysym = keysym;

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                for (k=0; k<16; k++)
                    if (old_ai.keyboard[k] == XK_VoidSymbol) {
                        /* We found an empty space to put our key*/
                        old_ai.keyboard[k] = ai.keyboard[i];
                        break;
                    }
            }

            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key %d.", events[evi].key.keysym.sym);
            evi++;

            /* Returning if we reached the asked number of events */
            if (evi == num)
                return evi;

        }
    }

    /* Returning the number of generated events */
    return evi;
}

/* TODO: Same, duplicate code */
int generateKeyDown1Event(SDL1_Event *events, int num, int update)
{
    int evi = 0;
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
            events[evi].type = SDL1_KEYDOWN;
            events[evi].key.which = 0; // FIXME: I don't know what is going here
            events[evi].key.state = SDL_PRESSED;

            SDL_keysym keysym;
            xkeysymToSDL1(&keysym, ai.keyboard[i]);
            events[evi].key.keysym = keysym;

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                for (k=0; k<16; k++)
                    if (old_ai.keyboard[k] == XK_VoidSymbol) {
                        /* We found an empty space to put our key*/
                        old_ai.keyboard[k] = ai.keyboard[i];
                        break;
                    }
            }

            debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key %d.", events[evi].key.keysym.sym);
            evi++;

            /* Returning if we reached the asked number of events */
            if (evi == num)
                return evi;

        }
    }

    /* Returning the number of generated events */
    return evi;
}

/* Generate SDL2 GameController events */

int generateControllerEvent(SDL_Event* events, int num, int update)
{
    int evi = 0;
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
                events[evi].type = SDL_CONTROLLERAXISMOTION;
                events[evi].caxis.timestamp = SDL_GetTicks_real(); // TODO: maybe our timer instead
                events[evi].caxis.which = joyid[ji];
                events[evi].caxis.axis = axis;
                events[evi].caxis.value = ai.controller_axes[ji][axis];

                if (update) {
                    /* Upload the old AllInput struct */
                    old_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];
                }

                evi++;

                if (evi == num) {
                    /* Return the number of events */
                    return evi;
                }
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
                    events[evi].type = SDL_CONTROLLERBUTTONDOWN;
                    events[evi].cbutton.state = SDL_PRESSED;
                }
                else {
                    events[evi].type = SDL_CONTROLLERBUTTONUP;
                    events[evi].cbutton.state = SDL_RELEASED;
                }
                events[evi].cbutton.timestamp = SDL_GetTicks_real(); // TODO: maybe our timer instead
                events[evi].cbutton.which = joyid[ji];
                events[evi].cbutton.button = bi;

                if (update) {
                    /* Upload the old AllInput struct */
                    old_ai.controller_buttons[ji] ^= (1 << bi);
                }

                if (evi == num) {
                    /* Return the number of events */
                    return evi;
                }
            }
        }
    }

    /* We did not reached the asked number of events, returning the number */
    return evi;
}

/**
 *  Count the number of joysticks attached to the system right now
 */
/* Override */ int SDL_NumJoysticks(void)
{
    debuglog(LCF_SDL | LCF_JOYSTICK, "%s call", __func__);
    /* For now, we declare one joystick */
    //return 1;
    return 0;
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




