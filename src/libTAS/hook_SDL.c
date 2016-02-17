#include "hook_SDL.h"

int hook_SDL(void* SDL_handle) {

    *(void**)&SDL_GL_SwapWindow_real = dlsym(SDL_handle, "SDL_GL_SwapWindow");
    if (!SDL_GL_SwapWindow_real)
    {
        //log_err("Could not import symbol SDL_GL_SwapWindow.");
        return 0;
    }

    *(void**)&SDL_CreateWindow_real = dlsym(SDL_handle, "SDL_CreateWindow");
    if (!SDL_CreateWindow_real)
    {
        //log_err("Could not import symbol SDL_CreateWindow.");
        return 0;
    }

    *(void**)&SDL_GetWindowID_real = dlsym(SDL_handle, "SDL_GetWindowID");
    if (!SDL_GetWindowID_real)
    {
        //log_err("Could not import symbol SDL_GetWindowID.");
        return 0;
    }

    *(void**)&SDL_GetWindowFlags_real = dlsym(SDL_handle, "SDL_GetWindowFlags");
    if (!SDL_GetWindowFlags_real)
    {
        //log_err("Could not import symbol SDL_GetWindowFlags.");
        return 0;
    }

    *(void**)&SDL_PollEvent_real = dlsym(SDL_handle, "SDL_PollEvent");
    if (!SDL_PollEvent_real)
    {
        //log_err("Could not import symbol SDL_PollEvent.");
        return 0;
    }

    *(void**)&SDL_PeepEvents_real = dlsym(SDL_handle, "SDL_PeepEvents");
    if (!SDL_PeepEvents_real)
    {
        //log_err("Could not import symbol SDL_PeepEvents.");
        return 0;
    }

    *(void**)&SDL_GetTicks_real = dlsym(SDL_handle, "SDL_GetTicks");
    if (!SDL_GetTicks_real)
    {
        //log_err("Could not import symbol SDL_GetTicks.");
        return 0;
    }

    return 1;
}

