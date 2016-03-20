#ifndef OPENGL_H_INCL
#define OPENGL_H_INCL

#ifdef LIBTAS_HUD

#include "../external/SDL_ttf.h"

void link_opengl(void);

void RenderText(TTF_Font *font, const char* message, int sw, int sh, SDL_Color color, int x, int y);

#endif
#endif
