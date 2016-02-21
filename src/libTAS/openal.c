#include "openal.h"


void* alcOpenDevice(const char* devicename)
{
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

    const char device[] = "No Output";
    return alcOpenDevice_real(device);
    //return alcOpenDevice_real(devicename);
}

