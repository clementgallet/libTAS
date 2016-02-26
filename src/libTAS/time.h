#include "logging.h"
#include "hook.h"

typedef long time_t;
typedef long suseconds_t;
typedef unsigned long useconds_t;
struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};

void advanceFrame(void);

/* Override */ int usleep(useconds_t usec);
/* Override */ time_t time(time_t* t);
/* Override */ int gettimeofday(struct timeval* tv, void* tz);
/* Override */ void SDL_Delay(Uint32 sleep);
/* Override */ Uint32 SDL_GetTicks(void);

