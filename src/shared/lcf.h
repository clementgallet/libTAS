/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_LCF_H_INCL
#define LIBTAS_LCF_H_INCL

/* Log level, should be combined with a log category */
typedef unsigned int LogLevel; enum
{
    LL_FATAL, // things that shouldn't happen and that we won't recover.
    LL_ERROR, // things that shouldn't happen.
    LL_WARN , // things that users should be warned about.
    LL_INFO , // things that users may want to know about.
    LL_DEBUG, // things that devs may want to know about.
    LL_TRACE, // reserved for logging each hooked function call.
    LL_STACK, // print additional backtrace.
    LL_SIZE ,
};

const char* const LL_NAMES[] = { "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE", "STACK" };
const char* const LL_COLORS[] = { "\x1b[31m", "\x1b[31m", "\x1b[33m", "\x1b[37m", "\x1b[32m", "\x1b[34m", "\x1b[94m" };

typedef unsigned int LogCategoryFlag; enum
{
    LCF_NONE     = 0,

    /* Special categories */
    LCF_MAINTHREAD = 1 << 0, // things that are executed by the main thread.
    /* UNUSED    = 1 << 1, */
    /* UNUSED    = 1 << 2, */
    /* UNUSED    = 1 << 3, */
    /* UNUSED    = 1 << 4, */
    LCF_TODO     = 1 << 5, // things known to require further implementation in the future.

    /* Log category */
    LCF_HOOK     = 1 << 6, // hooking notifications
    /* UNUSED   = 1 << 7, */
    LCF_TIMESET  = 1 << 8, // notifications of setting the internal time
    LCF_TIMEGET  = 1 << 9, // notifications of getting the internal time
    LCF_CHECKPOINT  = 1 << 10, // savestates
    LCF_WAIT     = 1 << 11, // waiting on synchronization objects
    LCF_SLEEP    = 1 << 12, // calls on thread sleep
    LCF_SOCKET   = 1 << 13, // sending messages over the socket
    LCF_LOCALE   = 1 << 14, // localization
    LCF_OGL      = 1 << 15, // openGL related functions
    LCF_VULKAN   = 1 << 15, // Vulkan related functions
    LCF_DUMP     = 1 << 16, // video and audio dumping
    LCF_SDL      = 1 << 17, // call to SDL functions
    LCF_WINE     = 1 << 18, // Wine functions
    LCF_KEYBOARD = 1 << 19, // keyboard related
    LCF_MOUSE    = 1 << 20,
    LCF_JOYSTICK = 1 << 21, // for both SDL_Joystick and SDL_GameController
    LCF_SYSTEM   = 1 << 22,
    LCF_SOUND    = 1 << 23,
    LCF_RANDOM   = 1 << 24,
    LCF_SIGNAL   = 1 << 25, // signals between processes/threads
    LCF_EVENTS   = 1 << 26, // processing SDL events
    LCF_WINDOW   = 1 << 27,
    LCF_FILEIO   = 1 << 28,
    LCF_STEAM    = 1 << 29, // Steam API
    LCF_THREAD   = 1 << 30,
    LCF_TIMERS   = 1 << 31, // as in async timer objects
    LCF_ALL      = 0xffffffff & ~(LCF_MAINTHREAD | LCF_TODO),
};

#endif
