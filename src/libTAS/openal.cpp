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

    (void) devicename; // To remove warning
    const char device[] = "No Output";
    return alcOpenDevice_real(device);
    //return alcOpenDevice_real(devicename);
}

