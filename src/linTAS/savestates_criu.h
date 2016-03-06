#ifndef SAVESTATES_CRIU_H_INCLUDED
#define SAVESTATES_CRIU_H_INCLUDED

#include <stdlib.h>
#include <fcntl.h>
#include "../external/criu/lib/c/criu.h"

void init_criu(int pid);
int dump_criu(void);
int restore_criu(void);

#endif

