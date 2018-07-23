/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_OPENALC_H_INCL
#define LIBTAS_OPENALC_H_INCL

#include "../../global.h"

/** 8-bit boolean */
typedef char ALCboolean;

/** character */
typedef char ALCchar;

/** signed 8-bit 2's complement integer */
typedef signed char ALCbyte;

/** unsigned 8-bit integer */
typedef unsigned char ALCubyte;

/** signed 16-bit 2's complement integer */
typedef short ALCshort;

/** unsigned 16-bit integer */
typedef unsigned short ALCushort;

/** signed 32-bit 2's complement integer */
typedef int ALCint;

/** unsigned 32-bit integer */
typedef unsigned int ALCuint;

/** non-negative 32-bit binary integer size */
typedef int ALCsizei;

/** enumerated 32-bit value */
typedef int ALCenum;

/** 32-bit IEEE754 floating-point */
typedef float ALCfloat;

/** 64-bit IEEE754 floating-point */
typedef double ALCdouble;

/** void type (for opaque pointers only) */
typedef void ALCvoid;


/* Enumerant values begin at column 50. No tabs. */

/** Boolean False. */
#define ALC_FALSE                                0

/** Boolean True. */
#define ALC_TRUE                                 1

/** Context attribute: <int> Hz. */
#define ALC_FREQUENCY                            0x1007

/** Context attribute: <int> Hz. */
#define ALC_REFRESH                              0x1008

/** Context attribute: AL_TRUE or AL_FALSE. */
#define ALC_SYNC                                 0x1009

/** Context attribute: <int> requested Mono (3D) Sources. */
#define ALC_MONO_SOURCES                         0x1010

/** Context attribute: <int> requested Stereo Sources. */
#define ALC_STEREO_SOURCES                       0x1011

/** No error. */
#define ALC_NO_ERROR                             0

/** Invalid device handle. */
#define ALC_INVALID_DEVICE                       0xA001

/** Invalid context handle. */
#define ALC_INVALID_CONTEXT                      0xA002

/** Invalid enum parameter passed to an ALC call. */
#define ALC_INVALID_ENUM                         0xA003

/** Invalid value parameter passed to an ALC call. */
#define ALC_INVALID_VALUE                        0xA004

/** Out of memory. */
#define ALC_OUT_OF_MEMORY                        0xA005


/** Runtime ALC version. */
#define ALC_MAJOR_VERSION                        0x1000
#define ALC_MINOR_VERSION                        0x1001

/** Context attribute list properties. */
#define ALC_ATTRIBUTES_SIZE                      0x1002
#define ALC_ALL_ATTRIBUTES                       0x1003

/** String for the default device specifier. */
#define ALC_DEFAULT_DEVICE_SPECIFIER             0x1004
/**
 * String for the given device's specifier.
 *
 * If device handle is NULL, it is instead a null-char separated list of
 * strings of known device specifiers (list ends with an empty string).
 */
#define ALC_DEVICE_SPECIFIER                     0x1005
/** String for space-separated list of ALC extensions. */
#define ALC_EXTENSIONS                           0x1006


/** Capture extension */
#define ALC_EXT_CAPTURE 1
/**
 * String for the given capture device's specifier.
 *
 * If device handle is NULL, it is instead a null-char separated list of
 * strings of known capture device specifiers (list ends with an empty string).
 */
#define ALC_CAPTURE_DEVICE_SPECIFIER             0x310
/** String for the default capture device specifier. */
#define ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER     0x311
/** Number of sample frames available for capture. */
#define ALC_CAPTURE_SAMPLES                      0x312


/** Enumerate All extension */
#define ALC_ENUMERATE_ALL_EXT 1
/** String for the default extended device specifier. */
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
/**
 * String for the given extended device's specifier.
 *
 * If device handle is NULL, it is instead a null-char separated list of
 * strings of known extended device specifiers (list ends with an empty string).
 */
#define ALC_ALL_DEVICES_SPECIFIER                0x1013


/* Create dummy types */
typedef int ALCdevice;
typedef int ALCcontext;

namespace libtas {

/** Device management. */
OVERRIDE ALCdevice* alcOpenDevice(const ALCchar* devicename);
OVERRIDE ALCboolean alcCloseDevice(ALCdevice* deviceHandle);

/** Context management. */
OVERRIDE ALCcontext* alcCreateContext(ALCdevice *device, const ALCint* attrlist);
OVERRIDE ALCboolean  alcMakeContextCurrent(ALCcontext *context);
OVERRIDE void        alcProcessContext(ALCcontext *context);
OVERRIDE void        alcSuspendContext(ALCcontext *context);
OVERRIDE void        alcDestroyContext(ALCcontext *context);
OVERRIDE ALCcontext* alcGetCurrentContext(void);
OVERRIDE ALCdevice*  alcGetContextsDevice(ALCcontext *context);

/**
 * Error support.
 *
 * Obtain the most recent Device error.
 */
OVERRIDE ALCenum alcGetError(ALCdevice *device);

/**
 * Extension support.
 *
 * Query for the presence of an extension, and obtain any appropriate
 * function pointers and enum values.
 */
OVERRIDE ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname);
OVERRIDE void*      alcGetProcAddress(ALCdevice *device, const ALCchar *funcname);
OVERRIDE ALCenum    alcGetEnumValue(ALCdevice *device, const ALCchar *enumname);

/** Query function. */
OVERRIDE const ALCchar* alcGetString(ALCdevice *device, ALCenum param);
OVERRIDE void           alcGetIntegerv(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values);

/** Extensions **/
OVERRIDE ALCboolean  myalcSetThreadContext(ALCcontext *context);
OVERRIDE ALCcontext* myalcGetThreadContext(void);

#define AL_DIRECT_CHANNELS_SOFT                  0x1033
#define ALC_FORMAT_CHANNELS_SOFT                 0x1990
#define ALC_FORMAT_TYPE_SOFT                     0x1991

/* Sample types */
#define ALC_BYTE_SOFT                            0x1400
#define ALC_UNSIGNED_BYTE_SOFT                   0x1401
#define ALC_SHORT_SOFT                           0x1402
#define ALC_UNSIGNED_SHORT_SOFT                  0x1403
#define ALC_INT_SOFT                             0x1404
#define ALC_UNSIGNED_INT_SOFT                    0x1405
#define ALC_FLOAT_SOFT                           0x1406

/* Channel configurations */
#define ALC_MONO_SOFT                            0x1500
#define ALC_STEREO_SOFT                          0x1501
#define ALC_QUAD_SOFT                            0x1503
#define ALC_5POINT1_SOFT                         0x1504
#define ALC_6POINT1_SOFT                         0x1505
#define ALC_7POINT1_SOFT                         0x1506

OVERRIDE ALCdevice* myalcLoopbackOpenDeviceSOFT(const ALCchar *deviceName);
OVERRIDE ALCboolean myalcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type);
OVERRIDE void myalcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples);

}

#endif
