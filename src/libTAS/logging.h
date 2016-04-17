/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "hook.h" // For pthread_self_real
#include "time.h" // For frame_counter
#include "ThreadState.h"
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

/* Convert an integer into a short string to allow meaningless ints
 * to be printed in a shorter representation and be easier to
 * recognize. In practice, it is base64 convertion.
 * Used by pthread ids.
 */
std::string stringify(unsigned long int id);

/* Main function to print the debug message str (or not), and additional
 * information, based on the LogCategoryFlag value
 */
void debuglogverbose(LogCategoryFlag lcf, std::string str, std::string& outstr);

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
 * depend on variadic templates is transfered to debuglogverbose(),
 * to keep increased size as low as possible.
 */
template<typename ...Args>
void debuglog(LogCategoryFlag lcf, Args ...args);
template<typename ...Args>
inline void debuglog(LogCategoryFlag lcf, Args ...args)
{
    /* Not printing anything if thread state is set to NOLOG */
    if (threadState.isNoLog())
        return;

    /* We avoid recursive loops by protecting eventual recursive calls to debuglog
     * in the following code
     */
    threadState.setNoLog(true);
    std::ostringstream oss;
    catlog(oss, std::forward<Args>(args)...);
    std::string outstr;
    debuglogverbose(lcf, oss.str(), outstr);
    std::cerr << outstr;
    threadState.setNoLog(false);
}

/* Print the debug message using stdio functions */
void debuglogstdio(LogCategoryFlag lcf, const char* fmt, ...);

/* If we only want to print the function name... */
#define DEBUGLOGCALL(lcf) debuglog(lcf, __func__, " call.")

#endif

