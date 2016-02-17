#ifndef HOOKSDL_H_INCLUDED
#define HOOKSDL_H_INCLUDED

#include <dlfcn.h>
#include "SDL.h"

void(* SDL_GL_SwapWindow_real)(void);
void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32 (* SDL_GetWindowID_real)(void*);
int (*SDL_PollEvent_real)(SDL_Event*);
int (*SDL_PeepEvents_real)(SDL_Event*, int, SDL_eventaction, Uint32, Uint32);
Uint32 (*SDL_GetTicks_real)(void);
Uint32 (*SDL_GetWindowFlags_real)(void*);

int hook_SDL(void* SDL_handle);

#endif // HOOKSDL_H_INCLUDED
