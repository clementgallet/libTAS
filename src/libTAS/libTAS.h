#ifndef LIBTAS_H_INCLUDED
#define LIBTAS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "SDL.h"
#include "keyboard.h"
#include "hook.h"
#include "../shared/messages.h"
#include "dumpvideo.h"
#include "time.h"

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

Uint8 keyboard_state[SDL_NUM_SCANCODES] = {0};

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);
void SDL_GL_SwapWindow(void);
void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
//Uint32 SDL_GetWindowId(void* window);
Uint32 SDL_GetWindowFlags(void* window);
const Uint8* SDL_GetKeyboardState(int* numkeys);
int SDL_PollEvent(SDL_Event* event);
int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
int SDL_GL_SetSwapInterval(int interval);
void SDL_DestroyWindow(void* window);
void SDL_Quit(void);
void proceed_commands(void);

#endif // LIBTAS_H_INCLUDED
