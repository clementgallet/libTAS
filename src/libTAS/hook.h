#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#include "../external/SDL.h"

extern int SDLver;

#define LINK_SUFFIX(FUNC,LIB) link_function((void**)&FUNC##_real, #FUNC, LIB)
bool link_function(void** function, const char* source, const char* library);
int get_sdlversion(void);

#endif // HOOKSDL_H_INCLUDED
