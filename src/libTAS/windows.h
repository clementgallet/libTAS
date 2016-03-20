#ifndef WINDOWS_H_INCL
#define WINDOWS_H_INCL

#include "global.h"
#include "../external/SDL.h"

extern void* gameWindow;
extern Uint32 (*SDL_GetWindowID_real)(void*);

OVERRIDE void SDL_GL_SwapWindow(void* window);
OVERRIDE void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
OVERRIDE Uint32 SDL_GetWindowID(void* window);
OVERRIDE Uint32 SDL_GetWindowFlags(void* window);
OVERRIDE int SDL_GL_SetSwapInterval(int interval);
OVERRIDE void SDL_DestroyWindow(void* window);

OVERRIDE SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
OVERRIDE void SDL_GL_SwapBuffers(void);

void link_sdlwindows(void);

#endif

