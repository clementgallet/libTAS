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
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <string.h>

namespace libtas {

/* Actual implementation with file and line */
void debuglogfull(LogCategoryFlag lcf, const char* file, int line, ...);

/* Print the debug message using stdio functions */
#define debuglogstdio(lcf, ...) debuglogfull(lcf, __FILE__, __LINE__, __VA_ARGS__)

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
