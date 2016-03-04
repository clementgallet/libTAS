#ifndef LIBTAS_H_INCLUDED
#define LIBTAS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "../external/SDL.h"
#include "../external/SDL_ttf.h"
#include "keyboard_helper.h"
#include "hook.h"
#include "../shared/messages.h"
#include "../shared/inputs.h"
#ifdef LIBTAS_DUMP
#include "dumpvideo.h"
#endif
#include "time.h"
#include "inputs.h"
#include "opengl.h"
#include "frame.h"
#include "socket.h"

#define MAGIC_NUMBER 42

extern unsigned long frame_counter;

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);

/* Override */ void SDL_Init(unsigned int flags);
/* Override */ int SDL_InitSubSystem(Uint32 flags);
/* Override */ void SDL_Quit(void);
/* Override */ void SDL_GL_SwapWindow(void* window);
/* Override */ void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
//Uint32 SDL_GetWindowId(void* window);
/* Override */ Uint32 SDL_GetWindowFlags(void* window);
/* Override */ int SDL_PollEvent(SDL_Event* event);
/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
/* Override */ int SDL_GL_SetSwapInterval(int interval);
/* Override */ void SDL_DestroyWindow(void* window);

#endif // LIBTAS_H_INCLUDED
