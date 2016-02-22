#ifndef LCF_H_INCL
#define LCF_H_INCL

/* Taken from the Hourglass code. Some categories will obviously not be used. */
typedef int LogCategoryFlag; enum
{
	LCF_NONE     = 0,
	LCF_UNKNOWN  = LCF_NONE,
	LCF_UNTESTED = 1 << 0, // things that have significant but not-yet-tested implementations. should be combined with other flags.
	LCF_DESYNC   = 1 << 1, // things considered worth inspecting in cases of desync. should be combined with other flags.
	LCF_FREQUENT = 1 << 2, // things that tend to get called very often (such as more than once per frame). should be combined with other flags.
	LCF_ERROR    = 1 << 3, // things that shouldn't happen. should be combined with other flags.
	LCF_TODO     = 1 << 4, // things known to require further implementation in the future. should be combined with other flags.
	LCF_FRAME    = 1 << 5, // things associated with frame boundaries. should be combined with other flags.
	LCF_HOOK     = 1 << 6, // hooking notifications
	LCF_TIMEFUNC = 1 << 7, // time-related functions
	LCF_TIMESET  = 1 << 8, // notifications of setting the internal time
	LCF_TIMEGET  = 1 << 9, // notifications of getting the internal time
	LCF_SYNCOBJ  = 1 << 10, // creating or modifying synchronization objects
	LCF_WAIT     = 1 << 11, // waiting on synchronization objects
	LCF_SLEEP    = 1 << 12,
	LCF_SOCKET   = 1 << 13,
	LCF_D3D      = 1 << 14,
	LCF_OGL      = 1 << 15,
	LCF_GDI      = 1 << 16,
	LCF_SDL      = 1 << 17,
	LCF_DINPUT   = 1 << 18,
	LCF_KEYBOARD = 1 << 19,
	LCF_MOUSE    = 1 << 20,
	LCF_JOYPAD   = 1 << 21,
	LCF_OPENAL   = 1 << 22,
	LCF_WSOUND   = 1 << 23, // non-directsound sound output, like wavout
	LCF_PROCESS  = 1 << 24,
	LCF_MODULE   = 1 << 25, // DLL functions and COM object stuff too.
	LCF_EVENTS   = 1 << 26,
	LCF_WINDOW   = 1 << 27, // windows windows
	LCF_FILEIO   = 1 << 28,
	LCF_REGISTRY = 1 << 29, // windows registry or system properties
	LCF_THREAD   = 1 << 30,
	LCF_TIMERS   = 1 << 31, // as in async timer objects
};

#endif // LCF_H_INCL
