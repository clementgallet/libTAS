#ifndef LIBTAS_H_INCLUDED
#define LIBTAS_H_INCLUDED

#include "../external/SDL.h"
#include "global.h"

#define MAGIC_NUMBER 42

extern void* gameWindow;

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);

OVERRIDE void SDL_Init(unsigned int flags);
OVERRIDE int SDL_InitSubSystem(Uint32 flags);
OVERRIDE void SDL_Quit(void);
OVERRIDE void SDL_GL_SwapWindow(void* window);
OVERRIDE void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
//Uint32 SDL_GetWindowId(void* window);
OVERRIDE Uint32 SDL_GetWindowFlags(void* window);
OVERRIDE int SDL_GL_SetSwapInterval(int interval);
OVERRIDE void SDL_DestroyWindow(void* window);

OVERRIDE SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
OVERRIDE void SDL_GL_SwapBuffers(void);


#endif // LIBTAS_H_INCLUDED
