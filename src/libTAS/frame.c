#include "frame.h"

void enterFrameBoundary(void)
{
    /* Sleep until the end of the frame length if necessary */
    if (tasflags.running && !tasflags.fastforward)
        sleepEndFrame();

    /* Advance deterministic time by one frame */
    advanceFrame();

    sendMessage(MSGB_START_FRAMEBOUNDARY);
    sendData(&frame_counter, sizeof(unsigned long));

    if (tasflags.running && tasflags.speed_divisor > 1)
        usleep_real(1000000 * (tasflags.speed_divisor - 1) / 60);

    struct TasFlags oldflags = tasflags;
    proceed_commands();

    /* Do the rest of the stuff before finishing the frame */
    if (oldflags.fastforward != tasflags.fastforward)
        SDL_GL_SetSwapInterval_real(!tasflags.fastforward);

    /* We don't update AllInputs old_ai here. We update during the generation of events */

    ++frame_counter;
}

void proceed_commands(void)
{
    int message;
    while (1)
    {
        receiveData(&message, sizeof(int));

        switch (message)
        {
            case MSGN_TASFLAGS:
                receiveData(&tasflags, sizeof(struct TasFlags));
                break;

            case MSGN_END_FRAMEBOUNDARY:
                return;

            case MSGN_ALL_INPUTS:
                receiveData(&ai, sizeof(struct AllInputs));
                break;

        }
    }
}

