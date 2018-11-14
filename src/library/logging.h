/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GlobalState.h"
#include <cstdio>

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

/* Helper functions to concatenate different arguments arbitrary types into
 * a string stream. Because it uses variadic templates, its definition must
 * be visible by files that #include it, so the compiler knows for which
 * types it has to build a function.
 *
 * I'm not sure if it is the right choice, as it rapidly populates with
 * hundred of symbols and takes hundreds of kB in memory.
 * However, I think removing the -g debugger flag does save lot of memory.
 */
void catlog(std::ostringstream &oss);
inline void catlog(std::ostringstream&) {}

template<typename First, typename ...Rest>
void catlog (std::ostringstream &oss, First && first, Rest && ...rest);
template<typename First, typename ...Rest>
inline void catlog (std::ostringstream &oss, First && first, Rest && ...rest)
{
    oss << std::forward<First>(first);
    catlog(oss, std::forward<Rest>(rest)...);
}

/* Print a variable list of arguments and other information based on the
 * value of lcf compared to the values in tasflags.includeFlags
 * and tasflags.excludeFlags.
 *
 * This function is the one called by other source files.
 * It uses variadic templates so the above comment does apply here also.
 *
 * The content is kept as minimal as possible and everything that does not
 * depend on variadic templates is transfered to debuglogstdio(),
 * to keep increased size as low as possible.
 */
template<typename ...Args>
void debuglog(LogCategoryFlag lcf, Args ...args);
template<typename ...Args>
inline void debuglog(LogCategoryFlag lcf, Args ...args)
{
    /* We also check this in debuglogstdio(), but doing it here avoid building
     * all strings, because as a fraction of these will be printed.
     */
    if ((!(lcf & shared_config.includeFlags) ||
          (lcf & shared_config.excludeFlags)) &&
         !(lcf & LCF_ALERT))
        return;

    std::ostringstream oss;
    catlog(oss, std::forward<Args>(args)...);
    debuglogstdio(lcf, oss.str().c_str());
}

/* If we only want to print the function name... */
#define DEBUGLOGCALL(lcf) debuglogstdio(lcf, "%s call.", __func__)

/* Macro of an assert */
#define MYASSERT(term) if ((term)) {} \
    else {debuglogstdio(LCF_ERROR, "%s failed in %s", #term, __func__); \
    exit(1);}

/* We want to store and send error messages to the program so that they can be
 * shown on a dialog box. We also need some synchronization to access the set
 * of error messages.
 */
void setAlertMsg(const char* alert, int size);
bool getAlertMsg(std::string& alert);

}

#endif
