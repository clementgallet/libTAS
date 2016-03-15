#include "openal.h"
#include "logging.h"
#include "hook.h"

#define ALC_DEVICE_SPECIFIER                     0x1005

/* Override */ void* alcOpenDevice(const char* devicename)
{

    debuglog(LCF_OPENAL, __func__, " call.");
    late_openalhook();

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

    (void) devicename; // To remove warning
    const char device[] = "No Output";
    return alcOpenDevice_real(device);
    //return alcOpenDevice_real(devicename);
}

