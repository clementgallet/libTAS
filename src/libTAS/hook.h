#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#include "../external/SDL.h"
#include "dlhook.h"

extern int SDLver;

extern void (*SDL_Init_real)(unsigned int flags);
extern int (*SDL_InitSubSystem_real)(Uint32 flags);
extern void (*SDL_Quit_real)(void);

extern void (*SDL_GetVersion_real)(SDL_version* ver);

/* SDL 1.2 specific functions */

extern SDL_version * (*SDL_Linked_Version_real)(void);

#define LINK_SUFFIX(FUNC,LIB) link_function((void**)&FUNC##_real, #FUNC, LIB)
bool link_function(void** function, const char* source, const char* library);
int get_sdlversion(void);

#endif // HOOKSDL_H_INCLUDED
