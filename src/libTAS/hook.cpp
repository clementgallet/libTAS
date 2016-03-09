#include "hook.h"
#include "logging.h"

#define __USE_GNU
#include <dlfcn.h>

#define HOOK_FUNC_TARGET(FUNC,SOURCE,TARGET) *(void**)&TARGET = dlsym(SOURCE, #FUNC);\
    if (!TARGET)\
    {\
        debuglog(LCF_ERROR | LCF_HOOK, "Could not import symbol " #FUNC ".");\
        TARGET = NULL;\
    }

#define HOOK_FUNC(FUNC,SOURCE) HOOK_FUNC_TARGET(FUNC,SOURCE,FUNC##_real)

#define HOOK_GLFUNC(FUNC) HOOK_FUNC(FUNC,RTLD_NEXT)\
    if (!FUNC##_real)\
    {\
        if (!SDL_GL_GetProcAddress_real) {\
            debuglog(LCF_HOOK | LCF_OGL | LCF_SDL | LCF_ERROR, "SDL_GL_GetProcAddress is not available. Could not load " #FUNC ".");\
            FUNC##_real = NULL;\
        }\
        else {\
            *(void**)&FUNC##_real = SDL_GL_GetProcAddress_real(#FUNC);\
            if (!FUNC##_real)\
            {\
                debuglog(LCF_HOOK | LCF_OGL | LCF_SDL | LCF_ERROR, "Could not load function " #FUNC ".");\
            }\
        }\
    }

int SDLver;

void (*SDL_Init_real)(unsigned int flags);
int (*SDL_InitSubSystem_real)(Uint32 flags);
void (*SDL_Quit_real)(void);
void(* SDL_GL_SwapWindow_real)(void* window);
void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32 (* SDL_GetWindowID_real)(void*);
int (*SDL_PollEvent_real)(SDL_Event*);
void (*SDL_PumpEvents_real)(void);
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
int (*SDL1_PollEvent_real)(SDL1_Event*);
int (*SDL1_PeepEvents_real)(SDL1_Event*, int, SDL_eventaction, Uint32);



int hook_functions(void* SDL_handle) {

    HOOK_FUNC(SDL_GetVersion, SDL_handle)
    HOOK_FUNC(SDL_Linked_Version, SDL_handle);

    /* Determine SDL version */
    SDL_version ver = {0, 0, 0};
    if (SDL_GetVersion_real) {
        SDL_GetVersion_real(&ver);
    }
    else if (SDL_Linked_Version_real) {
        SDL_version *verp;
        verp = SDL_Linked_Version_real();
        ver = *verp;
    }

    /* We save the version major in an extern variable because we may need it elsewhere */
    SDLver = ver.major;

    if (ver.major == 0) {
        debuglog(LCF_ERROR | LCF_SDL | LCF_HOOK, "Could not get SDL version...");
        return 0;
    }
    else if (ver.major == 1) {
        debuglog(LCF_SDL | LCF_HOOK, "Detected SDL 1 lib (%d.%d.%d).", ver.major, ver.minor, ver.patch);
        /* Hooking SDL 1.2 specific functions */
        HOOK_FUNC(SDL_GL_SwapBuffers, SDL_handle)
        HOOK_FUNC(SDL_SetVideoMode, SDL_handle)
        /* These two functions have a different signature than the SDL2 version */
        HOOK_FUNC_TARGET(SDL_PollEvent, SDL_handle, SDL1_PollEvent_real)
        HOOK_FUNC_TARGET(SDL_PeepEvents, SDL_handle, SDL1_PeepEvents_real)
    }
    else if (ver.major == 2) {
        debuglog(LCF_SDL | LCF_HOOK, "Detected SDL 2 lib (%d.%d.%d).", ver.major, ver.minor, ver.patch);
        /* Hooking SDL 2 specific functions */
        HOOK_FUNC(SDL_GL_SwapWindow, SDL_handle)
            HOOK_FUNC(SDL_CreateWindow, SDL_handle)
            HOOK_FUNC(SDL_DestroyWindow, SDL_handle)
            HOOK_FUNC(SDL_GetWindowID, SDL_handle)
            HOOK_FUNC(SDL_GetWindowFlags, SDL_handle)
            HOOK_FUNC(SDL_GL_SetSwapInterval, SDL_handle)
            HOOK_FUNC(SDL_GL_GetDrawableSize, SDL_handle)
            HOOK_FUNC(SDL_GetWindowSurface, SDL_handle)
            HOOK_FUNC(SDL_GetWindowWMInfo, SDL_handle)
            HOOK_FUNC(SDL_GetPerformanceFrequency, SDL_handle)
            HOOK_FUNC(SDL_GetPerformanceCounter, SDL_handle)
            HOOK_FUNC(SDL_PollEvent, SDL_handle)
            HOOK_FUNC(SDL_PeepEvents, SDL_handle)
    }

    HOOK_FUNC(SDL_PumpEvents, SDL_handle)
    HOOK_FUNC(SDL_Init, SDL_handle)
    HOOK_FUNC(SDL_InitSubSystem, SDL_handle)
    HOOK_FUNC(SDL_Quit, SDL_handle)
    HOOK_FUNC(SDL_GetTicks, SDL_handle)
    HOOK_FUNC(usleep, RTLD_NEXT)
    HOOK_FUNC(alcGetString, RTLD_NEXT)
    HOOK_FUNC(alcOpenDevice, RTLD_NEXT)
    HOOK_FUNC(SDL_CreateThread, SDL_handle)
    HOOK_FUNC(SDL_WaitThread, SDL_handle)
    //HOOK_FUNC(SDL_DetachThread, SDL_handle)
    HOOK_FUNC(pthread_create, RTLD_NEXT)
    HOOK_FUNC(pthread_exit, RTLD_NEXT)
    HOOK_FUNC(pthread_join, RTLD_NEXT)
    HOOK_FUNC(pthread_detach, RTLD_NEXT)
    HOOK_FUNC(pthread_getname_np, RTLD_NEXT)
    HOOK_FUNC(pthread_tryjoin_np, RTLD_NEXT)
    HOOK_FUNC(pthread_timedjoin_np, RTLD_NEXT)
    HOOK_FUNC(pthread_self, RTLD_NEXT)
    HOOK_FUNC(SDL_LockSurface, SDL_handle)
    HOOK_FUNC(SDL_UnlockSurface, SDL_handle)
    HOOK_FUNC(SDL_GL_GetProcAddress, SDL_handle)
    HOOK_FUNC(SDL_CreateRGBSurface, SDL_handle)
    HOOK_FUNC(SDL_FreeSurface, SDL_handle)
    HOOK_FUNC(SDL_SetColorKey, SDL_handle)
    HOOK_FUNC(SDL_FillRect, SDL_handle)
    HOOK_FUNC(SDL_AddTimer, SDL_handle)
    HOOK_FUNC(SDL_RemoveTimer, SDL_handle)
    /* TODO: Add SDL 1.2 SetTimer */

    HOOK_FUNC(glReadPixels, RTLD_NEXT)
    HOOK_FUNC(glGenTextures, RTLD_NEXT)
    HOOK_FUNC(glBindTexture, RTLD_NEXT)
    HOOK_FUNC(glTexImage2D, RTLD_NEXT)
    HOOK_FUNC(glBegin, RTLD_NEXT)
    HOOK_FUNC(glEnd, RTLD_NEXT)
    HOOK_FUNC(glVertex2f, RTLD_NEXT)
    HOOK_FUNC(glTexCoord2f, RTLD_NEXT)
    HOOK_FUNC(glDeleteTextures, RTLD_NEXT)
    HOOK_FUNC(glEnable, RTLD_NEXT)
    HOOK_FUNC(glDisable, RTLD_NEXT)
    HOOK_FUNC(glVertexPointer, RTLD_NEXT)
    HOOK_FUNC(glDrawArrays, RTLD_NEXT)
    HOOK_FUNC(glMatrixMode, RTLD_NEXT)
    HOOK_FUNC(glPushMatrix, RTLD_NEXT)
    HOOK_FUNC(glPopMatrix, RTLD_NEXT)
    HOOK_FUNC(glLoadIdentity, RTLD_NEXT)
    HOOK_FUNC(glOrtho, RTLD_NEXT)
    HOOK_FUNC(glBlendFunc, RTLD_NEXT)
    HOOK_FUNC(glTexParameteri, RTLD_NEXT)
    HOOK_FUNC(glGetIntegerv, RTLD_NEXT)
    HOOK_FUNC(glGetBooleanv, RTLD_NEXT)


    return 1;
}

int late_hook(void) {

    static int inited = 0;
    if (inited) return 1;

    HOOK_GLFUNC(glReadPixels)
    HOOK_GLFUNC(glGenTextures)
    HOOK_GLFUNC(glBindTexture)
    HOOK_GLFUNC(glTexImage2D)
    HOOK_GLFUNC(glBegin)
    HOOK_GLFUNC(glEnd)
    HOOK_GLFUNC(glVertex2f)
    HOOK_GLFUNC(glTexCoord2f)
    HOOK_GLFUNC(glDeleteTextures)
    HOOK_GLFUNC(glEnable)
    HOOK_GLFUNC(glDisable)
    HOOK_GLFUNC(glVertexPointer)
    HOOK_GLFUNC(glDrawArrays)
    HOOK_GLFUNC(glMatrixMode)
    HOOK_GLFUNC(glPushMatrix)
    HOOK_GLFUNC(glPopMatrix)
    HOOK_GLFUNC(glLoadIdentity)
    HOOK_GLFUNC(glOrtho)
    HOOK_GLFUNC(glBlendFunc)
    HOOK_GLFUNC(glTexParameteri)
    HOOK_GLFUNC(glGetIntegerv)
    HOOK_GLFUNC(glGetBooleanv)

    inited = 1;
    return 1;
}
