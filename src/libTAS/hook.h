#ifndef HOOKSDL_H_INCLUDED
#define HOOKSDL_H_INCLUDED

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include "../external/SDL.h"
#include "../external/gl.h"
#include "logging.h"


#define HOOK_FUNC(FUNC,SOURCE) *(void**)&FUNC##_real = dlsym(SOURCE, #FUNC);\
    if (!FUNC##_real)\
    {\
        debuglog(LCF_ERROR | LCF_HOOK, "Could not import symbol " #FUNC ".");\
        FUNC##_real = NULL;\
    }

#define HOOK_GLFUNC(FUNC) HOOK_FUNC(FUNC,RTLD_NEXT)\
    if (!FUNC##_real)\
    {\
        if (!SDL_GL_GetProcAddress_real) {\
            debuglog(LCF_HOOK | LCF_OGL | LCF_SDL | LCF_ERROR, "SDL_GL_GetProcAddress is not available. Could not load " #FUNC ".");\
            FUNC##_real = NULL;\
        }\
        else {\
            FUNC##_real = SDL_GL_GetProcAddress_real(#FUNC);\
            if (!FUNC##_real)\
            {\
                debuglog(LCF_HOOK | LCF_OGL | LCF_SDL | LCF_ERROR, "Could not load function " #FUNC ".");\
            }\
        }\
    }

#if (!defined __timespec_defined)
# define __timespec_defined 1
struct timespec
  {
    time_t tv_sec; /* Seconds.  */
    long tv_nsec;  /* Nanoseconds.  */
  };
#endif


void (*SDL_Init_real)(unsigned int flags);
int (*SDL_InitSubSystem_real)(Uint32 flags);
void (*SDL_Quit_real)(void);
void(* SDL_GL_SwapWindow_real)(void* window);
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
void* (*SDL_GL_GetProcAddress_real)(const char* proc);
void (*SDL_GetVersion_real)(SDL_version* ver);
SDL_bool (*SDL_GetWindowWMInfo_real)(void* window, SDL_SysWMinfo* info);
SDL_Surface* (*SDL_CreateRGBSurface_real)
    (Uint32 flags, int width, int height, int depth,
     Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
void (*SDL_FreeSurface_real)(SDL_Surface * surface);
int (*SDL_SetColorKey_real)(SDL_Surface * surface, int flag, Uint32 key);
int (*SDL_FillRect_real)(SDL_Surface * dst, const SDL_Rect * rect, Uint32 color);
int (*SDL_BlitSurface_real)(SDL_Surface*    src,
                       const SDL_Rect* srcrect,
                       SDL_Surface*    dst,
                       SDL_Rect*       dstrect);



void (*glReadPixels_real)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);
void (*glGenTextures_real)(int n, unsigned int* tex);
void (*glBindTexture_real)(int target, unsigned int tex);
void (*glTexImage2D_real)( int target,
    int level,
    int internalFormat,
    int width,
    int height,
    int border,
    int format,
    int type,
    const void * data);
void (*glBegin_real)( int mode );
void (*glEnd_real)( void );
void (*glVertex2f_real)( float x, float y );
void (*glTexCoord2f_real)( float s, float t );
void (*glDeleteTextures_real)( int n, const unsigned int *textures);
void (*glEnable_real)( int cap );
void (*glDisable_real)( int cap );
void (*glVertexPointer_real)(int size, int type, int stride, const void* pointer);
void (*glDrawArrays_real)( int mode, int first, int count);

void (*glMatrixMode_real)(int mode);
void (*glPushMatrix_real)(void);
void (*glPopMatrix_real)(void);
void (*glLoadIdentity_real)(void);
void (*glOrtho_real)(double left, double right, double bottom, double top, double near, double far);
void (*glBlendFunc_real)(int sfactor, int dfactor);
void (*glTexParameteri_real)(int target, int pname, int param);
void (*glGetIntegerv_real)( int pname, GLint* data);
void (*glGetBooleanv_real)( int pname, GLboolean* data);

int hook_functions(void* SDL_handle);
int late_hook(void);

#endif // HOOKSDL_H_INCLUDED
