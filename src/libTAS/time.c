#include "time.h"

/* Time that will be passed to the game 
 * Increments exactly by 1/fps after each screen draw
 */
struct timeval deterministic_time = { 0, 0 };

/* Real time structure, used to run at a given fps */
struct timespec real_time = { 0, 0 };

/* Advance the deterministic timer by exactly 1/fps */
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

    deterministic_time.tv_usec += integer_increment;
    fractional_part += fractional_increment;
    if (fractional_part >= fps)
    {
        ++deterministic_time.tv_usec;
        fractional_part -= fps;
    }

    if (deterministic_time.tv_usec == 1000000)
    {
        ++deterministic_time.tv_sec;
        deterministic_time.tv_usec = 0;
    }
}

/* Subtract correctly two timespec
 * Taken from http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 */

int timespec_subtract (struct timespec *res, struct timespec *x, struct timespec *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
    y->tv_nsec -= 1000000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_nsec - y->tv_nsec > 1000000000) {
    int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
    y->tv_nsec += 1000000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  res->tv_sec = x->tv_sec - y->tv_sec;
  res->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

/* 
 * When the game is running normally, we need to make our own sleep
 * so that the game runs at normal speed
 */
void sleepEndFrame(void)
{
    if ((real_time.tv_sec == 0) && (real_time.tv_nsec == 0)) {
        /* 
         * This is our first call 
         * We should not sleep, just update the real_time struct
         */
        clock_gettime(CLOCK_MONOTONIC, &real_time);
        return;
    }
    struct timespec old_time = real_time;

    /* Get the current time
     * We use CLOCK_MONOTONIC as it is not influenced by
     * timezone changes, leap seconds or so
     * Taken from https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time
     */
    clock_gettime(CLOCK_MONOTONIC, &real_time);

    /* We add the length of a frame to the old time */
    unsigned int fps = 60; // TODO: Put this somewhere else
    old_time.tv_nsec += 1000000000 / fps;

    struct timespec diff;
    int neg = timespec_subtract(&diff, &old_time, &real_time);

    /* Checking that we sleep for a positive time */
    if (!neg) {

        /* Sleeping */
        nanosleep(&diff, NULL);
    }
    /* Updating again the real_time */
    clock_gettime(CLOCK_MONOTONIC, &real_time);
}

/* Override */ time_t time(time_t* t)
{
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "%s call - returning %d.", __func__, (long)deterministic_time.tv_sec);
    if (t)
        *t = deterministic_time.tv_sec;
    return deterministic_time.tv_sec;
}

/* Override */ int gettimeofday(struct timeval* tv, __attribute__ ((unused)) void* tz)
{
    debuglog(LCF_TIMEGET | LCF_FREQUENT, "%s call - returning (%d,%d).", __func__, (long)deterministic_time.tv_sec, (long)deterministic_time.tv_usec);
    *tv = deterministic_time;
    return 0;
}

/* Override */ void SDL_Delay(unsigned int sleep)
{
    debuglog(LCF_SDL | LCF_SLEEP | LCF_FRAME, "%s call - sleep for %u ms.", __func__, sleep);
    if (sleep > 10 && sleep < 20) // TODO: Very hacky for now
        enterFrameBoundary();
    else
        usleep_real(sleep*1000);
}

/* Override */ int usleep(useconds_t usec)
{
    debuglog(LCF_SLEEP | LCF_FRAME, "%s call - sleep for %u us.", __func__, (unsigned int)usec);
    return usleep_real(usec);
}

/* Override */ Uint32 SDL_GetTicks(void)
{
    Uint32 msec = deterministic_time.tv_sec*1000 + deterministic_time.tv_usec/1000;
    debuglog(LCF_SDL | LCF_TIMEGET | LCF_FRAME, "%s call - true: %d, returning %d.", __func__, SDL_GetTicks_real(), msec);
    //return SDL_GetTicks_real();
    
    /*
     * We implement a fail-safe code here:
     * Some games expect SDL_GetTicks to increment, and will hang if not (Volgarr).
     * If the game called this function too many times and get the same result,
     * then we return the value + 1.
     */
    static int numcall = 0;
    static Uint32 oldmsec;
    if (msec == oldmsec) // TODO: oldmsec is undefined, but it is no big deal...
        numcall++;
    else
        numcall = 0;
    oldmsec = msec;
    if (numcall > 100) {
        numcall = 0;
        return msec + 1;
    }
    return msec;
}

