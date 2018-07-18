// To be run with LD_LIBRARY_PATH=. ./hookmain
// For libTAS, both runpath and libpath must be set to the program directory

#include <stdio.h>
#include <dlfcn.h>
#include "hooklib1.h"

int main()
{
    int ret;
    printf("Hooking a static function\n");
    ret = libtasTestFunc1();
    if (ret == 2) {
        printf("Successfully hooked!\n");
    }
    else {
        printf("Hooking failed!\n");
    }

    printf("Hooking a dynamic function\n");
    void* handle = dlopen("./libhooklib2.so", RTLD_LAZY);
    int (*func)() = (int (*)()) dlsym(handle, "libtasTestFunc2");

    if (!func) {
        printf("Cound not link to function libtasTestFunc2!\n");
    }

    ret = func();
    if (ret == 2) {
        printf("Successfully hooked!\n");
    }
    else {
        printf("Hooking failed!\n");
    }

    printf("Hooking a static function called by a dynamic function\n");
    int (*func2)() = (int (*)()) dlsym(handle, "libtasTestCallingFunc3");

    if (!func2) {
        printf("Cound not link to function libtasTestCallingFunc3!\n");
    }

    ret = func2();
    if (ret == 2) {
        printf("Successfully hooked!\n");
    }
    else {
        printf("Hooking failed!\n");
    }


}
