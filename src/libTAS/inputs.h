#ifndef INPUTS_H_INCL
#define INPUTS_H_INCL

#include <X11/keysym.h>
#include "SDL.h"
#include "keyboard_helper.h"
#include "logging.h"
#include "hook.h"
#include "../shared/inputs.h"

extern struct AllInputs ai;
extern struct AllInputs old_ai;

/* Override */ Uint8* SDL_GetKeyboardState(int* numkeys);
int generateKeyUpEvent(SDL_Event *event, void* gameWindow);
int generateKeyDownEvent(SDL_Event *event, void* gameWindow);

#endif // INPUTS_H_INCL
