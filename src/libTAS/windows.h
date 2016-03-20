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

#ifndef WINDOWS_H_INCL
#define WINDOWS_H_INCL

#include "global.h"
#include "../external/SDL.h"

extern void* gameWindow;
extern Uint32 (*SDL_GetWindowID_real)(void*);

OVERRIDE void SDL_GL_SwapWindow(void* window);
OVERRIDE void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
OVERRIDE Uint32 SDL_GetWindowID(void* window);
OVERRIDE Uint32 SDL_GetWindowFlags(void* window);
OVERRIDE int SDL_GL_SetSwapInterval(int interval);
OVERRIDE void SDL_DestroyWindow(void* window);

OVERRIDE SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
OVERRIDE void SDL_GL_SwapBuffers(void);

void link_sdlwindows(void);

#endif

