/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_LOGGING_H_INCL
#define LIBTAS_LOGGING_H_INCL

#include "../shared/lcf.h"
#include "global.h" // shared_config
#include "checkpoint/ThreadManager.h" // isMainThread()
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GlobalState.h"
#include <cstdio>
#include <string.h>

/* Color printing
 * Taken from http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
 */
#define ANSI_COLOR_RED           "\x1b[31m"
#define ANSI_COLOR_GREEN         "\x1b[32m"
#define ANSI_COLOR_YELLOW        "\x1b[33m"
#define ANSI_COLOR_BLUE          "\x1b[34m"
#define ANSI_COLOR_MAGENTA       "\x1b[35m"
#define ANSI_COLOR_CYAN          "\x1b[36m"
#define ANSI_COLOR_GRAY          "\x1b[37m"

#define ANSI_COLOR_LIGHT_RED     "\x1b[91m"
#define ANSI_COLOR_LIGHT_GREEN   "\x1b[92m"
#define ANSI_COLOR_LIGHT_YELLOW  "\x1b[93m"
#define ANSI_COLOR_LIGHT_BLUE    "\x1b[94m"
#define ANSI_COLOR_LIGHT_MAGENTA "\x1b[95m"
#define ANSI_COLOR_LIGHT_CYAN    "\x1b[96m"
#define ANSI_COLOR_LIGHT_GRAY    "\x1b[97m"

#define ANSI_COLOR_RESET         "\x1b[0m"

namespace libtas {

/* Print the debug message using stdio functions */
void debuglogstdio(LogCategoryFlag lcf, const char* fmt, ...);

/* If we only want to print the function name... */
#define DEBUGLOGCALL(lcf) debuglogstdio(lcf, "%s call.", __func__)

/* Macro of an assert */
#define MYASSERT(term) if ((term)) {} \
    else {debuglogstdio(LCF_ERROR, "%s failed in %s with error %s", #term, __func__, (errno == 0)?"None":strerror(errno)); \
    exit(1);}

/* Send error messages to the program so that they can be
 * shown on a dialog box. */
void sendAlertMsg(const std::string alert);

}

#endif
