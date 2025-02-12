// Compile with `gcc filebacking.c -lSDL2 -o filebacking`

#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Title",SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
   
    int run = 1;
    
    FILE *stream = fopen("file.txt", "w");
    char buffer[1024];
    sprintf(buffer, "0");
    fwrite(buffer, 1024, 1, stream);
    
    int framecount = 0;
    while (run) {
        fseek(stream, 0, SEEK_SET);
        fread(buffer, 1024, 1, stream);
        printf("Stored framecount: %s\n", buffer);
        
        sprintf(buffer, "%d", framecount);
        fseek(stream, 0, SEEK_SET);
        fwrite(buffer, 1024, 1, stream);
        printf("New stored framecount: %s\n", buffer);
        
        framecount++;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    run=0;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            run=0;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}
