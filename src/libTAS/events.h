#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include "../external/SDL.h"

int filterSDL1Event(SDL1_Event *event);
int filterSDL2Event(SDL_Event *event);
void logEvent(SDL_Event *event);

#endif

