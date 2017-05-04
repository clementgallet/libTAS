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

#include "logging.h"
#include <stdlib.h>
#include "threads.h"
#include "unistd.h" // For isatty
#include <cstdarg>
#include <cstring>
#include "hook.h" // For pthread_self_real
#include "time.h" // For frame_counter

void debuglogverbose(LogCategoryFlag lcf, const std::string& str, std::string &outstr)
{
    std::ostringstream oss;

    /* We only print colors if displayed on a terminal */
    bool isTerm = isatty(/*cerr*/ 2);
    if (isTerm) {
        if (lcf & LCF_ERROR)
            /* Write the header text in red */
            oss << ANSI_COLOR_RED;
        else if (lcf & LCF_TODO)
            /* Write the header text in light red */
            oss << ANSI_COLOR_LIGHT_RED;
        else
            /* Write the header text in white */
            oss << ANSI_COLOR_LIGHT_GRAY;
    }
    oss << "[libTAS f:" << frame_counter << "] ";

    std::string thstr = stringify(getThreadId());
    if (isMainThread())
        oss << "Thread " << thstr << " (main) ";
    else
        oss << "Thread " << thstr << "        ";

    if (isTerm) {
        /* Reset color change */
        oss << ANSI_COLOR_RESET;
    }

    if (lcf & LCF_ERROR)
        oss << "ERROR: ";

    /* Output arguments */
    oss << str << std::endl;

    outstr = oss.str();
}

void debuglogstdio(LogCategoryFlag lcf, const char* fmt, ...)
{
    /* Not printing anything if thread state is set to NOLOG */
    if (ThreadState::isNoLog())
        return;

    if ( !(lcf & shared_config.includeFlags) || (lcf & shared_config.excludeFlags) )
        return;

    /* We avoid recursive loops by protecting eventual recursive calls to debuglog
     * in the following code
     */
     ThreadNoLog tnl;

    /* Build main log string */
    int maxsize = 2048;
    char s[2048] = {'\0'};
    int size = 0;

    /* We only print colors if displayed on a terminal */
    bool isTerm = isatty(/*cerr*/ 2);
    if (isTerm) {
        if (lcf & LCF_ERROR)
            /* Write the header text in red */
            strncat(s, ANSI_COLOR_RED, maxsize-size-1);
        else if (lcf & LCF_TODO)
            /* Write the header text in light red */
            strncat(s, ANSI_COLOR_LIGHT_RED, maxsize-size-1);
        else
            /* Write the header text in white */
            strncat(s, ANSI_COLOR_LIGHT_GRAY, maxsize-size-1);
    }
    size = strlen(s);

    snprintf(s + size, maxsize-size-1, "[libTAS f:%ld] ", frame_counter);
    size = strlen(s);

    if (isTerm) {
        /* Reset color change */
        strncat(s, ANSI_COLOR_RESET, maxsize-size-1);
        size = strlen(s);
    }

    if (lcf & LCF_ERROR) {
        strncat(s, "ERROR: ", maxsize-size-1);
        size = strlen(s);
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(s + size, maxsize-size-1, fmt, args);
    va_end(args);
    size = strlen(s);

    strncat(s, "\n", maxsize-size-1);

    fprintf(stderr, s);
}

/* Print long integers as string for shorter ids. Use base64 */
std::string stringify(unsigned long int id)
{
    std::ostringstream oss;
    while (id) {
        unsigned long digit = id % 64;
        if (digit < 26) oss << static_cast<char>('A' + digit);
        else if (digit < 52) oss << static_cast<char>('a' + (digit - 26));
        else if (digit < 62) oss << static_cast<char>('0' + (digit - 52));
        else if (digit == 62) oss << '+';
        else oss << '/';

        id /= 64;
    }
    return oss.str();
}
