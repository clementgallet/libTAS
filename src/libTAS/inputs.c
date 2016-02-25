#include "inputs.h"

struct AllInputs ai;
struct AllInputs old_ai;

Uint8 SDL_keyboard[SDL_NUM_SCANCODES] = {0};

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
