#ifndef FRAME_H_INCL
#define FRAME_H_INCL

#include "hook.h"
#include "../shared/tasflags.h"
#include "../shared/messages.h"
#include "inputs.h"
#include "socket.h"
#include "time.h"
#include "libTAS.h"


void enterFrameBoundary(void);
void proceed_commands(void);

#endif
