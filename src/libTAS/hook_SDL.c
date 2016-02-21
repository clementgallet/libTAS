#include "hook_SDL.h"

int hook_SDL(void* SDL_handle) {

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

    return 1;
}

