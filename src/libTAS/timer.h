#include <time.h>
#include "logging.h"
#include "hook.h"
#include "TimeHolder.h"

enum TimeCallType
{
    TIMETYPE_UNTRACKED = -1,
    TIMETYPE_TIME = 0,
    TIMETYPE_GETTIMEOFDAY,
    TIMETYPE_CLOCK,
    TIMETYPE_CLOCKGETTIME,
    TIMETYPE_SDLGETTICKS,
    TIMETYPE_SDLGETPERFORMANCECOUNTER,
    TIMETYPE_NUMTRACKEDTYPES
};

/* A simple timer that directly uses the system timer,
 * but is somewhat affected by fast-forward and frame advance.
 * It is not deterministic, but can be a good reference for
 * comparing against what the deterministic timer does.
 * (i.e. run the game first with this to see what the framerate is).
 *
 * If the game runs differently with this timer, then
 * probably this one is doing the more correct behavior,
 * especially if the chosen frame rate is wrong.
 * But many games will be unrecordable while using this timer.
 *
 * Code largely taken from Hourglass.
 */

class NonDeterministicTimer
{
public:

    /* Initialize the class members */
    void initialize(void);

    /* Update and return the time of the non deterministic timer */
    struct timespec getTicks(void);

    /* Function called when entering a frame boundary */
    void enterFrameBoundary(void);

    /* Function called when exiting a frame boundary,
     * to take into account the time spent in it
     */
    void exitFrameBoundary(void);

    /* Add a delay in the timer, and sleep */
    void addDelay(struct timespec delayTicks);

private:
    TimeHolder ticks;
    TimeHolder lasttime;
    pthread_t frameThreadId;
    TimeHolder lastEnterTime;
    TimeHolder lastExitTime;
};

extern NonDeterministicTimer nonDetTimer;


/* A timer that gives deterministic values, at least in the main thread.
 *
 * Deterministic means that calling FrameBoundary() and querying this timer
 * in the same order will produce the same stream of results,
 * independently of anything else like the system clock or CPU speed.
 *
 * The main thread is defined as the thread that called SDL_Init(),
 *
 * The trick to making this timer deterministic and still run at a normal speed is:
 * we define a frame rate beforehand and always tell the game it is running that fast
 * from frame to frame. Then we do the waiting ourselves for each frame (using system timer).
 * this also lets us support "fast forward" without changing the values the game sees.
 */

class DeterministicTimer
{
public:

    /* Initialize the class members */
    void initialize(void);

    /* Update and return the time of the deterministic timer */
    struct timespec getTicks(TimeCallType type);

    /* Function called when entering a frame boundary */
    void enterFrameBoundary(void);

    /* Function called when exiting a frame boundary */
    void exitFrameBoundary(void);

    /* Add a delay in the timer, and sleep */
	void addDelay(struct timespec delayTicks);

private:

    /* Number of getTicks calls of a non main thread */
    unsigned int getTimes;

    /* By how much time did we increment the timer */
    TimeHolder timeIncrement;

    /* Remainder when increasing the timer by 1/fps */
    unsigned int fractional_part;

    /* State of the deterministic timer */
    TimeHolder ticks;

    /* Timer value during the last frame boundary enter */
    TimeHolder lastEnterTicks;

    /* 
     * Real time of the last frame boundary enter.
     * Used for sleeping the right amount of time
     */
    TimeHolder lastEnterTime;

    /* Is the lastEnterTime value valid?
     * True except on the first frame
     */
    bool lastEnterValid;

    /* Accumulated delay */
    TimeHolder addedDelay;

    /* Some ticks that we are forced to advance.
     * TODO: document this
     */
    TimeHolder forceAdvancedTicks;

    /* Store if we enter a frame boundary from a draw or
     * from a sleep/wait.
     * TODO: separate sleep and wait
     */
    bool drawFB;

    /* Limit for each time-getting method before time auto-advances to avoid a freeze */
    unsigned int altGetTimes [TIMETYPE_NUMTRACKEDTYPES];
    unsigned int altGetTimeLimits [TIMETYPE_NUMTRACKEDTYPES];
};

extern DeterministicTimer detTimer;

