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

#include "openal.h"
#include "logging.h"
#include "hook.h"

char* (*alcGetString_real)(void*, int) = nullptr;
void* (*alcOpenDevice_real)(const char*) = nullptr;

void link_openal(void);
void link_openal(void)
{
    LINK_SUFFIX(alcOpenDevice, "openal");
    LINK_SUFFIX(alcGetString, "openal");
}


#define ALC_DEVICE_SPECIFIER                     0x1005

/* Override */ void* alcOpenDevice(const char* devicename)
{

    debuglog(LCF_OPENAL, __func__, " call.");
    link_openal();

    /*
    const char *devices; 
    const char *ptr; 

    devices = alcGetString_real(NULL, ALC_DEVICE_SPECIFIER);
    ptr = devices;

    while (*ptr)
    {
        printf("   %s\n", ptr);
        ptr += strlen(ptr) + 1;
    }*/

    //(void) devicename; // To remove warning
    //const char device[] = "No Output";
    //return alcOpenDevice_real(device);
    return alcOpenDevice_real(devicename);
}

