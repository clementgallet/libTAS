// Compile with `gcc alloc.c -lSDL2 -o alloc`

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
    SDL_Rect rect = {200, 200, 20, 20};
    
    size_t alloc_size = 2ULL * 1024ULL * 1024ULL * 1024ULL;
    int alloc_nb = 1;
    
    srand(time(NULL));
    for (int i = 0; i < alloc_nb; i++) {
        void* addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE | ((i%2)?PROT_EXEC:0), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr) {
            // memset(addr, 0xff, alloc_size);
            int* array = (int*)addr;
            for (size_t j = 0; j < alloc_size / 4; j += 2) {
                array[j] = rand();
            }
        }
    }
    
    while (run) {
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

/* Results for 2 GB

*** last commit

Compressed Save (data half random): 9.367163, 9.565253, 9.706370, 9.194755, 9.518979
HK Compressed Save: 4.387866, 4.413509, 4.422949, 4.521323, 4.504775

*** commit df451643991a0ff118c5a1801298b2df2ed9fafc

Uncompressed Save: 0.120924, 0.133859, 0.127987, 0.130416, 0.135052
HK Compressed Save: 5.888642, 6.036040, 6.269808, 6.128422, 6.159773
HK Compressed Load: 1.723864, 1.696120, 1.691629, 1.712415, 1.642686

*** commit a989a10e2ce853fa053438d21e62734dc308525b

Uncompressed Save: 13.249837, 13.102027, 13.203901, 13.254516, 12.722798
Uncompressed Load: 0.045232, 0.044892, 0.047735, 0.043207, 0.048183
Compressed Save (data filled with same value): 1.797026, 1.528333, 1.528458, 1.726695, 1.527592, 1.518946
Compressed Load: 1.097693, 1.097663, 1.291076, 1.103812, 1.167946
Compressed Save (data half random, 1.652 MB size): 10.198781, 10.301655, 9.375456, 9.831924, 9.865086
Compressed Load: 1.686617, 1.644304, 1.669672, 1.641407, 1.644771


*** commit c3db9f940fcdfad86b37d4aefcddcfae66fc936a ***

Uncompressed Save: 12.403536, 13.028206, 12.887375, 13.084571, 12.603542
Uncompressed Load: 0.095708, 0.098490, 0.099461, 0.100389, 0.102359
Compressed Save (data filled with same value): 1.676194, 1.664495, 1.672026, 1.680690, 1.678954
Compressed Load: 1.156961, 1.156739, 1.151973, 1.146958, 1.154637
Compressed Save (data half random, 1.689 MB size): 9.957120, 9.703442, 9.955679, 9.907704, 9.820789
Compressed Load: 1.978608, 1.891071, 1.888100, 1.886097, 1.895941




 */
