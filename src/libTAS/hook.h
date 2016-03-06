#ifndef HOOKSDL_H_INCLUDED
#define HOOKSDL_H_INCLUDED

#include "../external/SDL.h"
#include "../external/gl.h"


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

typedef unsigned long int pthread_t;

int (*pthread_create_real) (void * thread, void * attr, void * (* start_routine) (void *), void * arg);
void (*pthread_exit_real) (void *retval);
int (*pthread_join_real) (unsigned long int thread, void **thread_return);
int (*pthread_detach_real) (unsigned long int thread);
int (*pthread_getname_np_real)(unsigned long int thread, char *name, size_t len);
int (*pthread_tryjoin_np_real)(unsigned long int thread, void **retval);
int (*pthread_timedjoin_np_real)(unsigned long int thread, void **retval, const struct timespec *abstime);
pthread_t (*pthread_self_real)(void);

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
Uint64 (*SDL_GetPerformanceFrequency_real)(void);
Uint64 (*SDL_GetPerformanceCounter_real)(void);

typedef int SDL_TimerID;
typedef Uint32 (*SDL_NewTimerCallback)(Uint32 interval, void *param);
SDL_TimerID (*SDL_AddTimer_real)(Uint32 interval, SDL_NewTimerCallback callback, void *param);
SDL_bool (*SDL_RemoveTimer_real)(SDL_TimerID id);

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

/* SDL 1.2 specific functions */

SDL_Surface *(*SDL_SetVideoMode_real)(int width, int height, int bpp, Uint32 flags);
void (*SDL_GL_SwapBuffers_real)(void);
SDL_version * (*SDL_Linked_Version_real)(void);

int hook_functions(void* SDL_handle);
int late_hook(void);

#endif // HOOKSDL_H_INCLUDED
