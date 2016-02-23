#ifndef HOOKSDL_H_INCLUDED
#define HOOKSDL_H_INCLUDED

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include "SDL.h"
#include "logging.h"

#define HOOK_FUNC(FUNC,SOURCE) *(void**)&FUNC##_real = dlsym(SOURCE, #FUNC);\
    if (!FUNC##_real)\
    {\
        debuglog(LCF_ERROR | LCF_HOOK, "Could not import symbol " #FUNC ".");\
        return 0;\
    }


struct timespec;

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

char* (*alcGetString_real)(void* device, int params);
void* (*alcOpenDevice_real)(const char* devicename);

/* Threads */
void* (*SDL_CreateThread_real)(int(*fn)(void*),
                       const char*   name,
                       void*         data);
void (*SDL_WaitThread_real)(void* thread, int *status);
//void (*SDL_DetachThread_real)(void * thread);

int (*pthread_create_real) (void * thread, void * attr, void * (* start_routine) (void *), void * arg);
void (*pthread_exit_real) (void *retval);
int (*pthread_join_real) (unsigned long int thread, void **thread_return);
int (*pthread_detach_real) (unsigned long int thread);
int (*pthread_getname_np_real)(unsigned long int thread, char *name, size_t len);
int (*pthread_tryjoin_np_real)(unsigned long int thread, void **retval);
int (*pthread_timedjoin_np_real)(unsigned long int thread, void **retval, const struct timespec *abstime);

void (*SDL_GL_GetDrawableSize_real)(void* window, int* w, int* h);
void* (*SDL_GetWindowSurface_real)(void* window);
int (*SDL_LockSurface_real)(void* surface);
void (*SDL_UnlockSurface_real)(void* surface);


void (*glReadPixels_real)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);

int hook_functions(void* SDL_handle);

#endif // HOOKSDL_H_INCLUDED
