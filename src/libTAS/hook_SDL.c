#include "hook_SDL.h"

int hook_SDL(void* SDL_handle, const struct TasFlags tasflags) {

    *(void**)&SDL_GL_SwapWindow_real = dlsym(SDL_handle, "SDL_GL_SwapWindow");
    if (!SDL_GL_SwapWindow_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_GL_SwapWindow.");
        return 0;
    }

    *(void**)&SDL_CreateWindow_real = dlsym(SDL_handle, "SDL_CreateWindow");
    if (!SDL_CreateWindow_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_CreateWindow.");
        return 0;
    }

    *(void**)&SDL_GetWindowID_real = dlsym(SDL_handle, "SDL_GetWindowID");
    if (!SDL_GetWindowID_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_GetWindowID.");
        return 0;
    }

    *(void**)&SDL_GetWindowFlags_real = dlsym(SDL_handle, "SDL_GetWindowFlags");
    if (!SDL_GetWindowFlags_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_GetWindowFlags.");
        return 0;
    }

    *(void**)&SDL_PollEvent_real = dlsym(SDL_handle, "SDL_PollEvent");
    if (!SDL_PollEvent_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_PollEvent.");
        return 0;
    }

    *(void**)&SDL_PeepEvents_real = dlsym(SDL_handle, "SDL_PeepEvents");
    if (!SDL_PeepEvents_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_PeepEvents.");
        return 0;
    }

    *(void**)&SDL_GetTicks_real = dlsym(SDL_handle, "SDL_GetTicks");
    if (!SDL_GetTicks_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Could not import symbol SDL_GetTicks.");
        return 0;
    }

    *(void**)&SDL_GL_SetSwapInterval_real = dlsym(SDL_handle, "SDL_GL_SetSwapInterval");
    if (!SDL_GL_SetSwapInterval_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Cound not load SDL_GL_SetSwapInterval");
        return 0;
    }

    *(void**)&usleep_real = dlsym(RTLD_NEXT, "usleep");
    if (!usleep_real)
    {
        debuglog(LCF_ERROR | LCF_HOOK, tasflags, "Cound not load usleep");
        return 0;
    }

    return 1;
}

