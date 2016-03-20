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

#ifndef LCF_H_INCL
#define LCF_H_INCL

/* Taken from the Hourglass code. Some categories will obviously not be used. */
typedef int LogCategoryFlag; enum
{
	LCF_NONE     = 0,
	LCF_UNKNOWN  = LCF_NONE,

    /* Log status, should be combined with a log category */
	LCF_UNTESTED = 1 << 0, // things that have significant but not-yet-tested implementations.
	LCF_DESYNC   = 1 << 1, // things considered worth inspecting in cases of desync.
	LCF_FREQUENT = 1 << 2, // things that tend to get called very often (more than once per frame).
	LCF_ERROR    = 1 << 3, // things that shouldn't happen.
	LCF_TODO     = 1 << 4, // things known to require further implementation in the future.
	LCF_FRAME    = 1 << 5, // things that are called once every frame.

    /* Log category */
	LCF_HOOK     = 1 << 6, // hooking notifications
	LCF_TIMEFUNC = 1 << 7, // time-related functions
	LCF_TIMESET  = 1 << 8, // notifications of setting the internal time
	LCF_TIMEGET  = 1 << 9, // notifications of getting the internal time
	//LCF_SYNCOBJ  = 1 << 10, // creating or modifying synchronization objects
	LCF_WAIT     = 1 << 11, // waiting on synchronization objects
	LCF_SLEEP    = 1 << 12, // calls on thread sleep
	LCF_SOCKET   = 1 << 13, // sending messages over the socket
	//LCF_D3D      = 1 << 14,
	LCF_OGL      = 1 << 15, // openGL related functions
	LCF_DUMP     = 1 << 16, // video and audio dumping
	LCF_SDL      = 1 << 17, // call to SDL functions
	//LCF_DINPUT   = 1 << 18,
	LCF_KEYBOARD = 1 << 19, // keyboard related
	//LCF_MOUSE    = 1 << 20,
	LCF_JOYSTICK = 1 << 21, // for both SDL_Joystick and SDL_GameController
	LCF_OPENAL   = 1 << 22, // call to openAL functions
	//LCF_WSOUND   = 1 << 23, // non-directsound sound output, like wavout
	//LCF_PROCESS  = 1 << 24,
	//LCF_MODULE   = 1 << 25, // DLL functions and COM object stuff too.
	LCF_EVENTS   = 1 << 26, // processing SDL events
	LCF_WINDOW   = 1 << 27, // windows windows
	//LCF_FILEIO   = 1 << 28,
	LCF_STEAM    = 1 << 29, // Steam API
	LCF_THREAD   = 1 << 30,
	//LCF_TIMERS   = 1 << 31, // as in async timer objects
    LCF_ALL = -1,
};

#endif // LCF_H_INCL
