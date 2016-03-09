#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include "../external/SDL.h"

extern "C" int SDL_PollEvent(SDL_Event* event);
extern "C" int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, ...);
int getSDL2Events(SDL_Event *events, int numevents, int update, Uint32 minType, Uint32 maxType);
int getSDL1Events(SDL1_Event *events, int numevents, int update, Uint32 mask);
int filterSDL1Event(SDL1_Event *event);
int filterSDL2Event(SDL_Event *event);
void logEvent(SDL_Event *event);

#endif

