/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
//#include "PerfTimer.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <string.h>

namespace libtas {

/* Actual implementation with file and line */
void debuglogfull(LogLevel ll, LogCategoryFlag lcf, const char* file, int line, ...);

/* Main logging function */
#define LOG(ll, lcf, ...) do {\
/*    PerfTimerCall ptc(lcf); */ \
    debuglogfull(ll, lcf, __FILE__, __LINE__, __VA_ARGS__);\
    } while (0)

/* Trace logging for hooked functions where we only want to print the function name */
#define LOGTRACE(lcf) LOG(LL_TRACE, lcf, "%s call.", __func__)

/* Macro of an assert */
#define MYASSERT(term) if ((term)) {} \
    else {LOG(LL_ERROR, LCF_NONE, "%s failed in %s with error %s", #term, __func__, (errno == 0)?"None":strerror(errno));}

/* Send error messages to the program so that they can be
 * shown on a dialog box. */
void sendAlertMsg(const std::string alert);

}

#endif
