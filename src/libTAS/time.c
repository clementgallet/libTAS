#include "time.h"

struct timeval current_time = { 0, 0 };

void advanceFrame(void)
{
    /* Once the frame is drawn, we can increment the current time by 1/60 of a
       second. This does not give an integer number of microseconds though, so
       we have to keep track of the fractional part of the microseconds
       counter, and add a spare microsecond each time it's needed. */
    unsigned int fps = 60;
    unsigned int integer_increment = 1000000 / fps;
    unsigned int fractional_increment = 1000000 % fps;
    unsigned int fractional_part = fps / 2;

    current_time.tv_usec += integer_increment;
    fractional_part += fractional_increment;
    if (fractional_part >= fps)
    {
        ++current_time.tv_usec;
        fractional_part -= fps;
    }

    if (current_time.tv_usec == 1000000)
    {
        ++current_time.tv_sec;
        current_time.tv_usec = 0;
    }
}

time_t time(time_t* t)
{
    debuglog(LCF_TIMEGET, "%s call - returning %d.", __func__, (long)current_time.tv_sec);
    if (t)
        *t = current_time.tv_sec;
    return current_time.tv_sec;
}

int gettimeofday(struct timeval* tv, __attribute__ ((unused)) void* tz)
{
    debuglog(LCF_TIMEGET, "%s call - returning (%d,%d).", __func__, (long)current_time.tv_sec, (long)current_time.tv_usec);
    *tv = current_time;
    return 0;
}

void SDL_Delay(unsigned int sleep)
{
    debuglog(LCF_SDL | LCF_SLEEP, "%s call - sleep for %u ms.", __func__, sleep);
    usleep_real(sleep*1000);
}

int usleep(useconds_t usec)
{
    debuglog(LCF_SLEEP, "%s call - sleep for %u us.", __func__, (unsigned int)usec);
    return usleep_real(usec);
}

Uint32 SDL_GetTicks(void)
{
    Uint32 msec = current_time.tv_sec*1000 + current_time.tv_usec/1000;
    debuglog(LCF_SDL | LCF_TIMEGET, "%s call - returning %d.", __func__, msec);
    //return SDL_GetTicks_real();
    return msec;
}

