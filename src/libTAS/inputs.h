#ifndef INPUTS_H_INCL
#define INPUTS_H_INCL

#include "../external/SDL.h"
#include "global.h"

extern struct AllInputs ai;
extern struct AllInputs old_ai;

/* 
 * Declaring SDL_GameController struct to be simply an int
 * containing the controller id
 */
typedef int SDL_GameController;


OVERRIDE Uint8* SDL_GetKeyboardState(int* numkeys);
OVERRIDE Uint8* SDL_GetKeyState( int* numkeys);
int generateKeyUpEvent(void *events, void* gameWindow, int num, int update);
int generateKeyDownEvent(void *events, void* gameWindow, int num, int update);
int generateControllerEvent(SDL_Event* events, int num, int update);

OVERRIDE int SDL_NumJoysticks(void);
OVERRIDE SDL_bool SDL_IsGameController(int joystick_index);
OVERRIDE SDL_GameController *SDL_GameControllerOpen(int joystick_index);
OVERRIDE const char *SDL_GameControllerNameForIndex(int joystick_index);
OVERRIDE const char *SDL_GameControllerName(SDL_GameController *gamecontroller);
OVERRIDE SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);
OVERRIDE int SDL_GameControllerEventState(int state);
OVERRIDE Sint16 SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                                                SDL_GameControllerAxis axis);
OVERRIDE Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                 SDL_GameControllerButton button);
OVERRIDE void SDL_GameControllerClose(SDL_GameController *gamecontroller);



#endif // INPUTS_H_INCL
