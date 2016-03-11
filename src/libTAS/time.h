#ifndef TIME_H_INCL
#define TIME_H_INCL

#include <time.h>
#include <unistd.h>
#include "../external/SDL.h"
#include "hook.h"
#include "stdbool.h"
#include "global.h"

/* Monotonic system-wide clock.  */
//#   define CLOCK_MONOTONIC      1


//typedef long time_t;
//typedef long suseconds_t;
//typedef unsigned long useconds_t;
/*
# ifndef _STRUCT_TIMEVAL
#  define _STRUCT_TIMEVAL   1
struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};
# endif*/ /* struct timeval */


//int clock_gettime(int clock_id, struct timespec *tp);
//extern int nanosleep (const struct timespec *requested_time,
//                      struct timespec *remaining);

extern unsigned long frame_counter;

void advanceFrame(void);
void sleepEndFrame(void);
int timespec_subtract (struct timespec *res, struct timespec *x, struct timespec *y);

OVERRIDE int usleep(useconds_t usec);
OVERRIDE time_t time(time_t* t);
OVERRIDE int gettimeofday(struct timeval* tv, void* tz);
OVERRIDE void SDL_Delay(Uint32 sleep);
OVERRIDE Uint32 SDL_GetTicks(void);
OVERRIDE Uint64 SDL_GetPerformanceFrequency(void);
OVERRIDE Uint64 SDL_GetPerformanceCounter(void);
SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_NewTimerCallback callback, void *param);
SDL_bool SDL_RemoveTimer(SDL_TimerID id);


#endif
