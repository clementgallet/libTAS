#include "hook.h"

int hook_functions(void* SDL_handle) {

    HOOK_FUNC(SDL_Init, SDL_handle)
    HOOK_FUNC(SDL_Quit, SDL_handle)
    HOOK_FUNC(SDL_GL_SwapWindow, SDL_handle)
    HOOK_FUNC(SDL_CreateWindow, SDL_handle)
    HOOK_FUNC(SDL_GetWindowID, SDL_handle)
    HOOK_FUNC(SDL_GetWindowFlags, SDL_handle)
    HOOK_FUNC(SDL_PollEvent, SDL_handle)
    HOOK_FUNC(SDL_PeepEvents, SDL_handle)
    HOOK_FUNC(SDL_GetTicks, SDL_handle)
    HOOK_FUNC(SDL_GL_SetSwapInterval, SDL_handle)
    HOOK_FUNC(usleep, RTLD_NEXT)
    HOOK_FUNC(SDL_DestroyWindow, SDL_handle)
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
    HOOK_FUNC(SDL_GL_GetDrawableSize, SDL_handle)
    HOOK_FUNC(SDL_GetWindowSurface, SDL_handle)
    HOOK_FUNC(SDL_LockSurface, SDL_handle)
    HOOK_FUNC(SDL_UnlockSurface, SDL_handle)
    HOOK_FUNC(SDL_GL_GetProcAddress, SDL_handle)
    HOOK_FUNC(SDL_GetVersion, SDL_handle)
    HOOK_FUNC(SDL_GetWindowWMInfo, SDL_handle)
    HOOK_FUNC(SDL_CreateRGBSurface, SDL_handle)
    HOOK_FUNC(SDL_FreeSurface, SDL_handle)
    HOOK_FUNC(SDL_SetColorKey, SDL_handle)
    HOOK_FUNC(SDL_FillRect, SDL_handle)

    HOOK_FUNC(glReadPixels, RTLD_NEXT)
    HOOK_FUNC(glGenTextures, RTLD_NEXT)
    HOOK_FUNC(glBindTexture, RTLD_NEXT)
    HOOK_FUNC(glTexImage2D, RTLD_NEXT)
    HOOK_FUNC(glBegin, RTLD_NEXT)
    HOOK_FUNC(glEnd, RTLD_NEXT)
    HOOK_FUNC(glVertex2f, RTLD_NEXT)
    HOOK_FUNC(glTexCoord2f, RTLD_NEXT)
    HOOK_FUNC(glDeleteTextures, RTLD_NEXT)

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

    inited = 1;
    return 1;
}
