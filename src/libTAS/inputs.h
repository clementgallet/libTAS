#ifndef INPUTS_H_INCL
#define INPUTS_H_INCL

#include "../external/SDL.h"

extern struct AllInputs ai;
extern struct AllInputs old_ai;

/* 
 * Declaring SDL_GameController struct to be simply an int
 * containing the controller id
 */
typedef int SDL_GameController;


/* Override */ Uint8* SDL_GetKeyboardState(int* numkeys);
int generateKeyUpEvent(SDL_Event *event, void* gameWindow);
int generateKeyDownEvent(SDL_Event *event, void* gameWindow);
int generateControllerEvent(SDL_Event* event);

/* Override */ int SDL_NumJoysticks(void);
/* Override */ SDL_bool SDL_IsGameController(int joystick_index);
/* Override */ SDL_GameController *SDL_GameControllerOpen(int joystick_index);
/* Override */ const char *SDL_GameControllerNameForIndex(int joystick_index);
/* Override */ const char *SDL_GameControllerName(SDL_GameController *gamecontroller);
/* Override */ SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);
/* Override */ int SDL_GameControllerEventState(int state);
/* Override */ Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                                SDL_GameControllerAxis axis);
/* Override */ Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button);
/* Override */ void SDL_GameControllerClose(SDL_GameController *gamecontroller);



#endif // INPUTS_H_INCL
