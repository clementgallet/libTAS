#include "hook_SDL.h"

#define ALC_DEVICE_SPECIFIER                     0x1005

void* alcOpenDevice(const char* devicename);
