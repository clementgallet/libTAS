#ifndef LIBTAS_H_INCLUDED
#define LIBTAS_H_INCLUDED

#include "../external/SDL.h"

#define MAGIC_NUMBER 42

extern void* gameWindow;

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);

extern "C" void SDL_Init(unsigned int flags);
extern "C" int SDL_InitSubSystem(Uint32 flags);
extern "C" void SDL_Quit(void);
extern "C" void SDL_GL_SwapWindow(void* window);
extern "C" void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
//Uint32 SDL_GetWindowId(void* window);
extern "C" Uint32 SDL_GetWindowFlags(void* window);
extern "C" int SDL_GL_SetSwapInterval(int interval);
extern "C" void SDL_DestroyWindow(void* window);

extern "C" SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
extern "C" void SDL_GL_SwapBuffers(void);


#endif // LIBTAS_H_INCLUDED
