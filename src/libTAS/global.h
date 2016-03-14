/* Include this file in every source code that override functions of the game */

#if __GNUC__ >= 4
    #define OVERRIDE extern "C" __attribute__ ((visibility ("default")))
#else
    #define OVERRIDE extern "C"
#endif

