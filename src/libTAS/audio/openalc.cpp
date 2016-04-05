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

#include "openalc.h"
#include "../logging.h"
#include <iostream>

ALCdevice dummyDevice = 0;
ALCcontext dummyContext = -1;
ALCcontext currentContext = -1;

ALCenum alcError = ALC_NO_ERROR;
#define ALCSETERROR(error) if(alcError==ALC_NO_ERROR) alcError = error

/* Override */ ALCenum alcGetError(ALCdevice *device)
{
    DEBUGLOGCALL(LCF_OPENAL);
    ALCenum err = alcError;
    alcError = ALC_NO_ERROR;
    return err;
}

/* Override */ ALCdevice* alcOpenDevice(const ALCchar* devicename)
{
    DEBUGLOGCALL(LCF_OPENAL);
    return &dummyDevice;
}

/* Override */ ALCboolean alcCloseDevice(ALCdevice* deviceHandle)
{
    DEBUGLOGCALL(LCF_OPENAL);
    return ALC_TRUE;
}

/* Override */ ALCcontext* alcCreateContext(ALCdevice *device, const ALCint* attrlist)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (dummyContext != -1) {
        debuglog(LCF_OPENAL | LCF_TODO, "We don't support multiple openAL contexts yet");
        return NULL;
    }
    dummyContext = 0;
    return &dummyContext;
}

/* Override */ ALCboolean alcMakeContextCurrent(ALCcontext *context)
{
    DEBUGLOGCALL(LCF_OPENAL);

    if (context == NULL) {
        currentContext = -1;
        return ALC_TRUE;
    }

    if (*context != 0) {
        ALCSETERROR(ALC_INVALID_CONTEXT);
        return ALC_FALSE;
    }
    currentContext = *context;
    return ALC_TRUE;
}

/* Override */ void alcProcessContext(ALCcontext *context)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (context == NULL)
        ALCSETERROR(ALC_INVALID_CONTEXT);
    if (*context != dummyContext)
        ALCSETERROR(ALC_INVALID_CONTEXT);
}

/* Override */ void alcSuspendContext(ALCcontext *context)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (context == NULL)
        ALCSETERROR(ALC_INVALID_CONTEXT);
    if (*context != dummyContext)
        ALCSETERROR(ALC_INVALID_CONTEXT);
}

/* Override */ void alcDestroyContext(ALCcontext *context)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (context == NULL)
        ALCSETERROR(ALC_INVALID_CONTEXT);
    if (*context == dummyContext) {
        if (*context == currentContext) {
            ALCSETERROR(ALC_INVALID_VALUE);
        }
        else {
            dummyContext = -1;
        }
    }
    else
        ALCSETERROR(ALC_INVALID_CONTEXT);
}

/* Override */ ALCcontext* alcGetCurrentContext(void)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (currentContext == -1)
        return NULL;
    else
        return &dummyContext;

}

/* Override */ ALCdevice* alcGetContextsDevice(ALCcontext *context)
{
    DEBUGLOGCALL(LCF_OPENAL);
    return &dummyDevice;
}

/* Override */ ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (extname == NULL) {
        ALCSETERROR(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    debuglog(LCF_OPENAL, "Extension asked is ", extname);
    return ALC_FALSE;
}

/* Override */ void* alcGetProcAddress(ALCdevice *device, const ALCchar *funcname)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (funcname == NULL) {
        ALCSETERROR(ALC_INVALID_VALUE);
        return NULL;
    }
    
    debuglog(LCF_OPENAL | LCF_ERROR, "Requesting function ", funcname);
    return NULL;
}

/* Override */ ALCenum alcGetEnumValue(ALCdevice *device, const ALCchar *enumname)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (enumname == NULL) {
        ALCSETERROR(ALC_INVALID_VALUE);
        return 0;
    }
    
    debuglog(LCF_OPENAL | LCF_ERROR, "Requesting enum ", enumname);
    return 0;
}

const ALCchar* alcNoErrorStr = "No error";
const ALCchar* alcInvalidDeviceStr = "Invalid device";
const ALCchar* alcInvalidContextStr = "Invalid context";
const ALCchar* alcInvalidEnumStr = "Invalid enum";
const ALCchar* alcInvalidValueStr = "Invalid value";
const ALCchar* alcOutOfMemoryStr = "Out of memory";
const ALCchar* alcExtensionsStr = ""; // extensions strings separated by space
const ALCchar* alcDeviceListStr = "libTAS device\0"; // must be double null-terminated
const ALCchar* alcDeviceStr = "libTAS device";
const ALCchar* alcCaptureListStr = "\0"; // must be double null-terminated
const ALCchar* alcDefault = "";

/* Override */ const ALCchar* alcGetString(ALCdevice *device, ALCenum param)
{
    //debuglog(LCF_OPENAL, __func__, " call with param ", std::hex, param, std::dec);
    debuglog(LCF_OPENAL, __func__, " call with param ", param);

    switch (param) {
        case ALC_DEFAULT_DEVICE_SPECIFIER:
            debuglog(LCF_OPENAL, "Request default device");
            return alcDeviceStr;

        case ALC_DEVICE_SPECIFIER:
            if (device == NULL) {
                debuglog(LCF_OPENAL, "Request list of available devices");
                return alcDeviceListStr;
            }
            else {
                debuglog(LCF_OPENAL, "Request current device");
                return alcDeviceStr;
            }

        case ALC_EXTENSIONS:
            debuglog(LCF_OPENAL, "Request list of supported extensions");
            if (device == NULL) {
                ALCSETERROR(ALC_INVALID_DEVICE);
                return NULL;
            }
            return alcExtensionsStr;

        case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
            debuglog(LCF_OPENAL, "Request default capture device");
            return NULL;
        case ALC_CAPTURE_DEVICE_SPECIFIER:
            if (device == NULL) {
                debuglog(LCF_OPENAL, "Request list of available capture devices");
                return alcCaptureListStr;
            }
            else {
                debuglog(LCF_OPENAL, "Request current capture device");
                return NULL;
            }

        /* Request error strings */
        case ALC_NO_ERROR:
            return alcNoErrorStr;
        case ALC_INVALID_DEVICE:
            return alcInvalidDeviceStr;
        case ALC_INVALID_CONTEXT:
            return alcInvalidContextStr;
        case ALC_INVALID_ENUM:
            return alcInvalidEnumStr;
        case ALC_INVALID_VALUE:
            return alcInvalidValueStr;
        case ALC_OUT_OF_MEMORY:
            return alcOutOfMemoryStr;
        default:
            return alcDefault;
    }
}

void alcGetIntegerv(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);

    if (values == NULL)
        return;
    if (size == 0)
        return;

    switch (param) {
        case ALC_FREQUENCY:
            debuglog(LCF_OPENAL | LCF_TODO, "Request frequency");
            values[0] = 44100;
            return;
        case ALC_REFRESH:
            debuglog(LCF_OPENAL | LCF_TODO, "Request refresh");
            values[0] = 60;
            return;
        case ALC_SYNC:
            debuglog(LCF_OPENAL | LCF_TODO, "Request sync");
            values[0] = 0;
            return;
        case ALC_MONO_SOURCES:
            debuglog(LCF_OPENAL | LCF_TODO, "Request mono sources");
            values[0] = 0;
            return;
        case ALC_STEREO_SOURCES:
            debuglog(LCF_OPENAL | LCF_TODO, "Request stereo sources");
            values[0] = 0;
            return;
        case ALC_ATTRIBUTES_SIZE:
            debuglog(LCF_OPENAL | LCF_TODO, "Request attributes size");
            if (device == NULL)
            values[0] = 1;
            return;
        case ALC_ALL_ATTRIBUTES:
            debuglog(LCF_OPENAL | LCF_TODO, "Request all attributes");
            values[0] = 0;
            return;
        case ALC_MAJOR_VERSION:
            debuglog(LCF_OPENAL, "Request major version");
            values[0] = 1;
            return;
        case ALC_MINOR_VERSION:
            debuglog(LCF_OPENAL, "Request minor version");
            values[0] = 1;
            return;
        case ALC_CAPTURE_SAMPLES:
            debuglog(LCF_OPENAL | LCF_TODO, "Request capture samples");
            values[0] = 0;
            return;
    }
}

