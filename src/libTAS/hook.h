#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#include "../external/SDL.h"
#include "../external/gl.h"
#include "bits/pthreadtypes.h"
#include "dlhook.h"
#include <time.h>

#if (!defined __timespec_defined)
# define __timespec_defined 1
struct timespec
  {
    time_t tv_sec; /* Seconds.  */
    long tv_nsec;  /* Nanoseconds.  */
  };
#endif

extern int SDLver;

extern void (*SDL_Init_real)(unsigned int flags);
extern int (*SDL_InitSubSystem_real)(Uint32 flags);
extern void (*SDL_Quit_real)(void);
extern void(* SDL_GL_SwapWindow_real)(void* window);
extern void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
extern Uint32 (* SDL_GetWindowID_real)(void*);
extern int (*SDL_PollEvent_real)(SDL_Event*);
extern void (*SDL_PumpEvents_real)(void);
extern int (*SDL_PeepEvents_real)(SDL_Event*, int, SDL_eventaction, Uint32, Uint32);
extern Uint32 (*SDL_GetTicks_real)(void);
extern Uint32 (*SDL_GetWindowFlags_real)(void*);
extern int (*SDL_GL_SetSwapInterval_real)(int interval);
extern void (*SDL_DestroyWindow_real)(void*);
extern int (*usleep_real)(unsigned long);
extern int (*nanosleep_real) (const struct timespec *requested_time, struct timespec *remaining);
extern char* (*alcGetString_real)(void* device, int params);
extern void* (*alcOpenDevice_real)(const char* devicename);

/* Threads */
extern void* (*SDL_CreateThread_real)(int(*fn)(void*),
                       const char*   name,
                       void*         data);
extern void (*SDL_WaitThread_real)(void* thread, int *status);
//void (*SDL_DetachThread_real)(void * thread);

extern int (*pthread_create_real) (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg);

/* Standard _Noreturn c++11 tag does not seem to work with function pointers, so using the GCC attribute instead */
extern __attribute__((__noreturn__)) void (*pthread_exit_real) (void *retval);
extern int (*pthread_join_real) (unsigned long int thread, void **thread_return);
extern int (*pthread_detach_real) (unsigned long int thread);
extern int (*pthread_getname_np_real)(unsigned long int thread, char *name, size_t len);
extern int (*pthread_tryjoin_np_real)(unsigned long int thread, void **retval);
extern int (*pthread_timedjoin_np_real)(unsigned long int thread, void **retval, const struct timespec *abstime);

extern pthread_t (*pthread_self_real)(void);

extern void (*SDL_GL_GetDrawableSize_real)(void* window, int* w, int* h);
extern void* (*SDL_GetWindowSurface_real)(void* window);
extern int (*SDL_LockSurface_real)(void* surface);
extern void (*SDL_UnlockSurface_real)(void* surface);
extern void* (*SDL_GL_GetProcAddress_real)(const char* proc);
extern void (*SDL_GetVersion_real)(SDL_version* ver);
extern SDL_bool (*SDL_GetWindowWMInfo_real)(void* window, SDL_SysWMinfo* info);
extern SDL_Surface* (*SDL_CreateRGBSurface_real)
    (Uint32 flags, int width, int height, int depth,
     Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern void (*SDL_FreeSurface_real)(SDL_Surface * surface);
extern int (*SDL_SetColorKey_real)(SDL_Surface * surface, int flag, Uint32 key);
extern int (*SDL_FillRect_real)(SDL_Surface * dst, const SDL_Rect * rect, Uint32 color);
extern Uint64 (*SDL_GetPerformanceFrequency_real)(void);
extern Uint64 (*SDL_GetPerformanceCounter_real)(void);

extern int (*clock_gettime_real) (clockid_t clock_id, struct timespec *tp);

typedef int SDL_TimerID;

typedef Uint32 (*SDL_NewTimerCallback)(Uint32 interval, void *param);
extern SDL_TimerID (*SDL_AddTimer_real)(Uint32 interval, SDL_NewTimerCallback callback, void *param);
extern SDL_bool (*SDL_RemoveTimer_real)(SDL_TimerID id);

extern void (*glReadPixels_real)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);
extern void (*glGenTextures_real)(int n, unsigned int* tex);
extern void (*glBindTexture_real)(int target, unsigned int tex);
extern void (*glTexImage2D_real)( int target,
    int level,
    int internalFormat,
    int width,
    int height,
    int border,
    int format,
    int type,
    const void * data);
extern void (*glBegin_real)( int mode );
extern void (*glEnd_real)( void );
extern void (*glVertex2f_real)( float x, float y );
extern void (*glTexCoord2f_real)( float s, float t );
extern void (*glDeleteTextures_real)( int n, const unsigned int *textures);
extern void (*glEnable_real)( int cap );
extern void (*glDisable_real)( int cap );
extern void (*glVertexPointer_real)(int size, int type, int stride, const void* pointer);
extern void (*glDrawArrays_real)( int mode, int first, int count);
extern void (*glMatrixMode_real)(int mode);
extern void (*glPushMatrix_real)(void);
extern void (*glPopMatrix_real)(void);
extern void (*glLoadIdentity_real)(void);
extern void (*glOrtho_real)(double left, double right, double bottom, double top, double near, double far);
extern void (*glBlendFunc_real)(int sfactor, int dfactor);
extern void (*glTexParameteri_real)(int target, int pname, int param);
extern void (*glGetIntegerv_real)( int pname, GLint* data);
extern void (*glGetBooleanv_real)( int pname, GLboolean* data);

/* SDL 1.2 specific functions */

extern SDL_Surface *(*SDL_SetVideoMode_real)(int width, int height, int bpp, Uint32 flags);
extern void (*SDL_GL_SwapBuffers_real)(void);
extern SDL_version * (*SDL_Linked_Version_real)(void);
extern int (*SDL1_PollEvent_real)(SDL1::SDL_Event*);
extern int (*SDL1_PeepEvents_real)(SDL1::SDL_Event*, int, SDL_eventaction, Uint32);

int hook_functions(void* SDL_handle);
int late_openalhook(void);
int late_glhook(void);

#endif // HOOKSDL_H_INCLUDED
