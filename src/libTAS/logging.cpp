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
#include "threadwrappers.h"
#include <unistd.h> // For isatty
#include <cstdarg>
#include <cstring>
// #include "hook.h" // For pthread_self_real
#include "timewrappers.h" // For frame_counter
#include <mutex>
#include <list>

namespace libtas {

void debuglogstdio(LogCategoryFlag lcf, const char* fmt, ...)
{
    /* Not printing anything if global state is set to NOLOG */
    if (GlobalState::isNoLog())
        return;

    if ((!(lcf & shared_config.includeFlags)  ||
          (lcf & shared_config.excludeFlags)) &&
         !(lcf & LCF_ALERT))
        return;

    /* We avoid recursive loops by protecting eventual recursive calls to debuglog
     * in the following code
     */
     GlobalNoLog tnl;

    /* Build main log string */

    /* We avoid any memory allocation here, because some parts of our code
     * are critical about memory allocation, like checkpointing.
     */
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

    const char* thstr = stringify(getThreadId());
    if (isMainThread())
        snprintf(s + size, maxsize-size-1, "Thread %s (main) ", thstr);
    else
        snprintf(s + size, maxsize-size-1, "Thread %s        ", thstr);
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

    /* If we must send the string to the program for displaying to the user,
     * we only send the actual message */
    if (lcf & LCF_ALERT) {
        setAlertMsg(s+size, strlen(s)-size);
    }

    size = strlen(s);

    strncat(s, "\n", maxsize-size-1);

    fputs(s, stderr);

}

/* Print long integers as string for shorter ids. Use base64 */
const char* stringify(pthread_t id)
{
    static char string[17] = {0};
    int idx = 0;
    while (id && (idx<16)) {
        unsigned long digit = id % 64;
        if (digit < 26) string[idx++] = static_cast<char>('A' + digit);
        else if (digit < 52) string[idx++] = static_cast<char>('a' + (digit - 26));
        else if (digit < 62) string[idx++] = static_cast<char>('0' + (digit - 52));
        else if (digit == 62) string[idx++] = '+';
        else string[idx++] = '/';

        id /= 64;
    }
    string[idx] = '\0';
    return string;
}

static std::list<std::string> alert_messages;
static std::mutex mutex;

void setAlertMsg(const char* alert, int size)
{
    std::lock_guard<std::mutex> lock(mutex);
    alert_messages.push_back(std::string(alert, size));
}

bool getAlertMsg(std::string& alert)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (alert_messages.empty())
        return false;
    alert = alert_messages.front();
    alert_messages.pop_front();
    return true;
}

}
