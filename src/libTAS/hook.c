#include "hook.h"

int hook_functions(void* SDL_handle) {

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

    HOOK_FUNC(glReadPixels, RTLD_NEXT)
    return 1;
}

