/* This code offers different scenarios for memory mappings, to check if 
 * libTAS is able to optimize savestates by guessing when mappings are not 
 * needed to be saved in savestates
 *
 * Compile with `gcc mmap.c -lSDL2 -o mmap`
 */

#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Title",SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            640,
            480,
            SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
   
    
    const size_t block_size = 4096;
    
    /* Use a fixed address for mmap, so that we can identify our mappings 
     * easily inside /proc/self/maps */
    char* current_address = (char*)0x555500000000uL;
    
    /* Leave holes between each mapping, so that mappings are not merged */
    size_t guard_size = 4096;
    
    void* addr;
    
    /** Private anonymous mappings **/
    
    /* 1. Private anonymous block with one page written */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;
    memset(addr, 0x55, 4096);

    /* 2. Private anonymous block with written zeros */
    addr = mmap(current_address, block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;
    memset(addr, 0, 4096);

    /* 3. Private anonymous block left untouched with RW perm */
    addr = mmap(current_address, block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /* 4. Private anonymous block left untouched with R perm */
    addr = mmap(current_address, block_size, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /* 5. Private anonymous block left untouched with no perm */
    addr = mmap(current_address, block_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /** Private file mappings **/

    char chunk[block_size];
    memset(chunk, 0xff, block_size);
    
    int fd = open("dummy.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    for (int i=0; i < 20; i++)
        write(fd, chunk, block_size);

    /* 1. Private file mapping with one page written */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FILE, fd, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;
    memset(addr, 0x55, 4096);

    /* 2. Private file mapping left untouched with RW perm */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FILE, fd, 2*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /* 3. Private file mapping left untouched with R perm */
    addr = mmap(current_address, 2*block_size, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 4*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /* 4. Private file mapping left untouched with no perm */
    addr = mmap(current_address, 2*block_size, PROT_NONE, MAP_PRIVATE | MAP_FILE, fd, 6*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /** Shared anonymous mappings **/
    
    /* 1. Shared anonymous block with one page written */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;
    memset(addr, 0x55, 4096);

    /* 2. Shared anonymous block with written zeros */
    addr = mmap(current_address, block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;
    memset(addr, 0, 4096);

    /* 3. Shared anonymous block left untouched with RW perm */
    addr = mmap(current_address, block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /* 4. Shared anonymous block left untouched with R perm */
    addr = mmap(current_address, block_size, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /* 5. Shared anonymous block left untouched with no perm */
    addr = mmap(current_address, block_size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += block_size + guard_size;

    /** Shared file mappings **/

    /* 1. Shared file mapping with one page written */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 8*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;
    memset(addr, 0x55, 4096);

    /* 2. Shared file mapping left untouched with RW perm */
    addr = mmap(current_address, 2*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 10*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /* 3. Shared file mapping left untouched with R perm */
    addr = mmap(current_address, 2*block_size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 12*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /* 4. Shared file mapping left untouched with no perm */
    addr = mmap(current_address, 2*block_size, PROT_NONE, MAP_SHARED | MAP_FILE, fd, 14*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 2*block_size + guard_size;

    /* Create a file with holes */
    int fd2 = open("dummy_hole.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd2, 50*block_size);
    lseek(fd2, 0, SEEK_SET);
    write(fd2, chunk, block_size);
    lseek(fd2, -block_size, SEEK_END);    
    write(fd2, chunk, block_size);

    /* 5. Shared file mapping with a hole */
    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd2, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;

    /* Create a file inside tmpfs device /dev/shm */
    int fd3 = open("/dev/shm/dummy_hole.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd3, 50*block_size);
    lseek(fd3, 0, SEEK_SET);
    write(fd3, chunk, block_size);
    lseek(fd3, -block_size, SEEK_END);    
    write(fd3, chunk, block_size);

    /* 6. Shared shm file mapping with a hole */
    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd3, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;

    /* Create a second file inside tmpfs device /dev/shm */
    int fd32 = open("/dev/shm/dummy_hole2.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd32, 50*block_size);
    lseek(fd32, 0, SEEK_SET);
    write(fd32, chunk, block_size);
    lseek(fd32, -block_size, SEEK_END);    
    write(fd32, chunk, block_size);

    /* 6. Shared shm file mapping with a hole and delete file */
    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd32, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;
    unlink("/dev/shm/dummy_hole2.txt");

    /* 7. Multiple shared file mappings that partly intersect */
    int fd4 = open("dummy2.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    for (int i=0; i < 100; i++)
        write(fd4, chunk, block_size);

    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd4, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;

    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd4, 20*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;

    addr = mmap(current_address, 70*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd4, 10*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 70*block_size + guard_size;

    addr = mmap(current_address, 50*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd4, 50*block_size);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 50*block_size + guard_size;

    addr = mmap(current_address, 100*block_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd4, 0);
    if (addr == MAP_FAILED) { printf("mmap failed\n"); return 0;}
    current_address += 100*block_size + guard_size;


    close(fd);
    close(fd2);
    close(fd3);
    close(fd32);
    close(fd4);





    int run = 1;

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
