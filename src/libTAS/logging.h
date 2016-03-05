#ifndef LOGGING_H_INCL
#define LOGGING_H_INCL

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../shared/tasflags.h"
#include "../shared/lcf.h"
#include "hook.h"
#include "thread.h"

/* Color printing
 * Taken from http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
 */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GRAY    "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void debuglog(LogCategoryFlag lcf, const char* msg, ...);

#endif // LOGCATEGORY_H_INCL
