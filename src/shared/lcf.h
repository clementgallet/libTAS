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

#ifndef LIBTAS_LCF_H_INCL
#define LIBTAS_LCF_H_INCL

/* Taken from the Hourglass code.
 * Categories for printing debug messages.
 * Some categories will obviously not be used. */
typedef int LogCategoryFlag; enum
{
    LCF_NONE     = 0,

    /* Log status, should be combined with a log category */
    LCF_MAINTHREAD = 1 << 0, // things that are executed by the main thread.
    LCF_FREQUENT = 1 << 1, // things that tend to get called very often (more than once per frame).
    LCF_ERROR    = 1 << 2, // things that shouldn't happen.
    LCF_WARNING  = 1 << 3, // things that users should be warned about.
    LCF_INFO     = 1 << 4, // things that users may want to know about.
    LCF_TODO     = 1 << 5, // things known to require further implementation in the future.

    /* Log category */
    LCF_HOOK     = 1 << 6, // hooking notifications
    LCF_ALERT    = 1 << 7, // messages that must be shown to the user
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
    LCF_ALL      = -1,
};

#endif
