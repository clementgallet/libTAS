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

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"
#define SDLK_SPACE 32
#define SDLK_LSHIFT 304
#define SDLK_UP	273
#define SDLK_DOWN 274
#define SDLK_RIGHT 275
#define SDLK_LEFT 276

#define SDLK_LAST 323

#define log_err(s) fprintf(stderr, "[libTAS] %s\n", (s))

typedef unsigned char Uint8;
typedef long time_t;
typedef long suseconds_t;
typedef unsigned long useconds_t;
struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};
int usleep(useconds_t usec);

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);
time_t time(time_t* t);
int gettimeofday(struct timeval* tv, void* tz);
void SDL_GL_SwapWindow(void);
Uint8* SDL_GetKeyState(int* keynums);
int SDL_PollEvent(void* event);
void proceed_commands(void);
void record_inputs(void);
void replay_inputs(void);

#endif // LIBTAS_H_INCLUDED
