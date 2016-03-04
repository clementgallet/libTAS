#ifndef OPENAL_H_INCL
#define OPENAL_H_INCL

#include "hook.h"

#define ALC_DEVICE_SPECIFIER                     0x1005

/* Override */ void* alcOpenDevice(const char* devicename);

#endif
