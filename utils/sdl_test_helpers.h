#include <SDL2/SDL.h>

#define SDL_TEST_INIT() \
    SDL_Init(SDL_INIT_VIDEO); \
    SDL_Window* sdl_window = SDL_CreateWindow("Title",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN); \
    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window,-1,SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC)

#define SDL_TEST_FRAME(run) \
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255); \
    SDL_RenderClear(sdl_renderer); \
    SDL_RenderPresent(sdl_renderer); \
    SDL_Event event = {}; \
    do { \
        switch(event.type) { \
            case SDL_QUIT: \
                run=false; \
                break; \
            case SDL_KEYDOWN: \
                switch(event.key.keysym.sym) { \
                    case SDLK_ESCAPE: \
                        run=false; \
                        break; \
                    default: \
                        break; \
                } \
                break; \
        } \
    } while (SDL_PollEvent(&event))

#define SDL_TEST_QUIT() \
    SDL_DestroyRenderer(sdl_renderer); \
    SDL_DestroyWindow(sdl_window); \
    SDL_Quit()
