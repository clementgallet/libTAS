#include "frame.h"
#include "hook.h"
#include "../shared/tasflags.h"
#include "../shared/inputs.h"
#include "../shared/messages.h"
#include "inputs.h"
#include "socket.h"
#include "time.h"
#include "libTAS.h"
#include "logging.h"
#include "timer.h"

void frameBoundary(void)
{
    debuglog(LCF_TIMEFUNC | LCF_FRAME, "Enter frame boundary");

    detTimer.enterFrameBoundary();

    sendMessage(MSGB_START_FRAMEBOUNDARY);
    sendData(&frame_counter, sizeof(unsigned long));

    struct TasFlags oldflags = tasflags;
    proceed_commands();

    /* We don't update AllInputs old_ai here. We update during the generation of events */
    ++frame_counter;

    detTimer.exitFrameBoundary();
    debuglog(LCF_TIMEFUNC | LCF_FRAME, "Leave frame boundary");
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

