/* This code calls all functions to get the system time
 * Can be compiled with: g++ -o gettimes gettimes.cpp `pkg-config --libs --cflags sdl2`
 
 * functions altered by setting time: time, gettimeofday, clock_gettime with CLOCK_REALTIME, CLOCK_REALTIME_COARSE and CLOCK_TAI
 */

#include <SDL2/SDL.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int main()
{    
    while (true) {
        time_t tt = time(NULL);
        std::cout << "time() returns " << tt << std::endl;
        
        struct timeval tv;
        gettimeofday(&tv, NULL);
        std::cout << "gettimeofday() returns " << tv.tv_sec << " s and " << tv.tv_usec << " us" << std::endl;

        clock_t ct = clock();
        std::cout << "clock() returns " << ct << std::endl;

        struct timespec tp;
        clock_gettime (CLOCK_REALTIME, &tp);
        std::cout << "clock_gettime() with CLOCK_REALTIME returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_REALTIME_COARSE, &tp);
        std::cout << "clock_gettime() with CLOCK_REALTIME_COARSE returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_TAI, &tp);
        std::cout << "clock_gettime() with CLOCK_TAI returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_MONOTONIC, &tp);
        std::cout << "clock_gettime() with CLOCK_MONOTONIC returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_MONOTONIC_COARSE, &tp);
        std::cout << "clock_gettime() with CLOCK_MONOTONIC_COARSE returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_MONOTONIC_RAW, &tp);
        std::cout << "clock_gettime() with CLOCK_MONOTONIC_RAW returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_BOOTTIME, &tp);
        std::cout << "clock_gettime() with CLOCK_BOOTTIME returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_BOOTTIME_ALARM, &tp);
        std::cout << "clock_gettime() with CLOCK_BOOTTIME_ALARM returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &tp);
        std::cout << "clock_gettime() with CLOCK_PROCESS_CPUTIME_ID returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        clock_gettime (CLOCK_THREAD_CPUTIME_ID, &tp);
        std::cout << "clock_gettime() with CLOCK_THREAD_CPUTIME_ID returns " << tp.tv_sec << " s and " << tp.tv_nsec << " us" << std::endl;

        Uint32 sdlt = SDL_GetTicks();
        std::cout << "SDL_GetTicks() returns " << sdlt << std::endl;
        
        // Uint64 sdlt64 = SDL_GetTicks64();
        // std::cout << "SDL_GetTicks64() returns " << sdlt64 << std::endl;

        Uint64 sdlpc = SDL_GetPerformanceCounter();
        std::cout << "SDL_GetPerformanceCounter() returns " << sdlpc << std::endl;
        
        usleep(1000000);        
    }

    return 0;
}
