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

#include "logging.h"
#include "checkpoint/ThreadManager.h" // isMainThread()
#include "frame.h" // For framecount
#include "global.h" // Global::shared_config
#include "GlobalState.h"
#include "renderhud/LogWindow.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <stdlib.h>
#include <unistd.h> // For isatty
#include <cstdarg>
#include <cstring>
#include <inttypes.h> // PRI stuff
#include <mutex>
#include <list>

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

void debuglogfull(LogLevel ll, LogCategoryFlag lcf, const char* file, int line, ...)
{
    /* Check log level and categories */
    if (Global::shared_config.logging_level < ll)
        return;

    if (lcf & Global::shared_config.logging_exclude_flags)
        return;

    if ((lcf != LCF_NONE) && !(lcf & Global::shared_config.logging_include_flags))
        return;

    if ((Global::shared_config.logging_include_flags & LCF_MAINTHREAD) &&
        !ThreadManager::isMainThread())
        return;

    /* Not printing anything if global state is set to NOLOG */
    if (GlobalState::isNoLog())
        return;

    /* We avoid recursive loops by protecting eventual recursive calls to debuglog
     * in the following code
     */
    GlobalNoLog tnl;

    /* Sanitize values */
    if (ll > LL_TRACE)
        ll = LL_TRACE;

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
        if (ll <= LL_ERROR)
            /* Write the header text in red */
            strncat(s, ANSI_COLOR_RED, maxsize-size-1);
        else if (ll == LL_WARN)
            /* Write the header text in light red */
            strncat(s, ANSI_COLOR_LIGHT_RED, maxsize-size-1);
        else
            /* Write the header text in white */
            strncat(s, ANSI_COLOR_LIGHT_GRAY, maxsize-size-1);
    }
    size = strlen(s);

    pid_t tid;
    if (Global::is_fork)
        /* For forked processes, the thread manager have wrong pid values (those of parent process) */
        NATIVECALL(tid = getpid());
    else
        tid = ThreadManager::getThreadTid();

    snprintf(s + size, maxsize-size-1, "[f:%" PRIu64 " t:%d%s] ", framecount, tid, Global::is_fork?"F":(ThreadManager::isMainThread()?"M":""));

    /* We append the string in multiple parts to the log window twice, to
     * not show the color characters */
    if (!(lcf & LCF_CHECKPOINT))
        LogWindow::addLog(s + size, s + strlen(s), false);

    size = strlen(s);

    if (isTerm) {
        /* Reset color change */
        strncat(s, ANSI_COLOR_RESET, maxsize-size-1);
        size = strlen(s);
    }

    int beg_size = size;

    if (ll <= LL_ERROR) {
        snprintf(s + size, maxsize-size-1, "%s (%s:%d): ", LL_NAMES[ll], file, line);
    }
    else {
        snprintf(s + size, maxsize-size-1, "%s: ", LL_NAMES[ll]);
    }
    size = strlen(s);

    va_list args;
    va_start(args, line);
    char* fmt = va_arg(args, char *);
    vsnprintf(s + size, maxsize-size-1, fmt, args);
    va_end(args);

    size = strlen(s);

    strncat(s, "\n", maxsize-size-1);

    if (!(lcf & LCF_CHECKPOINT))
        LogWindow::addLog(s + beg_size, s + strlen(s), true);

    /* We need to use a non-locking function here, because of the following
     * situation (encountered in Towerfall):
     * - Thread 1 starts printing a debug message and acquire the lock
     * - Thread 2 sends a signal to thread 1, interrupting the printing
     * - Thread 1 executes a function that is wrapped, and prints a debug message
     * - Thread 1 wants to acquire the lock
     */
#ifdef __linux__
    fputs_unlocked(s, stderr);
#else
    fputs(s, stderr);
#endif
}

void sendAlertMsg(const std::string alert)
{
    lockSocket();
    sendMessage(MSGB_ALERT_MSG);
    sendString(alert);
    unlockSocket();
}

}
