#ifndef LIBTAS_H_INCLUDED
#define LIBTAS_H_INCLUDED

#include "../external/SDL.h"
#include "global.h"

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);

OVERRIDE void SDL_Init(unsigned int flags);
OVERRIDE int SDL_InitSubSystem(Uint32 flags);
OVERRIDE void SDL_Quit(void);

#endif // LIBTAS_H_INCLUDED
