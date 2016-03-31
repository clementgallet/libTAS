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

#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#include "../external/SDL.h"

extern int SDLver;
extern void (*SDL_GetVersion_real)(SDL_version* ver);

#define LINK_SUFFIX(FUNC,LIB) link_function((void**)&FUNC##_real, #FUNC, LIB)
#define LINK_SUFFIX_SDL1(FUNC) LINK_SUFFIX(FUNC,"libSDL-1.2")
#define LINK_SUFFIX_SDL2(FUNC) LINK_SUFFIX(FUNC,"libSDL2-2")

bool link_function(void** function, const char* source, const char* library);
int get_sdlversion(void);

#endif // HOOKSDL_H_INCLUDED
