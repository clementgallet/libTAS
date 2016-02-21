#ifndef LOGGING_H_INCL
#define LOGGING_H_INCL

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../shared/tasflags.h"
#include "../shared/lcf.h"

void debuglog(LogCategoryFlag lcf, const char* msg, ...);

#endif // LOGCATEGORY_H_INCL
