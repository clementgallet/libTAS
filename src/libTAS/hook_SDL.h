#ifndef HOOKSDL_H_INCLUDED
#define HOOKSDL_H_INCLUDED

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include "SDL.h"
//#include "../shared/tasflags.h"
#include "logging.h"

void(* SDL_GL_SwapWindow_real)(void);
void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32 (* SDL_GetWindowID_real)(void*);
int (*SDL_PollEvent_real)(SDL_Event*);
int (*SDL_PeepEvents_real)(SDL_Event*, int, SDL_eventaction, Uint32, Uint32);
Uint32 (*SDL_GetTicks_real)(void);
Uint32 (*SDL_GetWindowFlags_real)(void*);
int (*SDL_GL_SetSwapInterval_real)(int interval);
void (*SDL_DestroyWindow_real)(void*);

int (*usleep_real)(unsigned long);

int hook_SDL(void* SDL_handle, struct TasFlags tasflags);

#endif // HOOKSDL_H_INCLUDED
