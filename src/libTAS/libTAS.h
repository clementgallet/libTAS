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

#ifndef LIBTAS_LIBTAS_H_INCLUDED
#define LIBTAS_LIBTAS_H_INCLUDED

#include <SDL2/SDL.h>
#include "global.h"
#include <string>

namespace libtas {

void __attribute__((constructor)) init(void);
void __attribute__((destructor)) term(void);

/**
 *  This function initializes the subsystems specified by \c flags
 */
OVERRIDE int SDL_Init(Uint32 flags);

/**
 *  This function initializes specific SDL subsystems
 *
 *  Subsystem initialization is ref-counted, you must call
 *  SDL_QuitSubSystem for each SDL_InitSubSystem to correctly
 *  shutdown a subsystem manually (or call SDL_Quit to force shutdown).
 *  If a subsystem is already loaded then this call will
 *  increase the ref-count and return.
 */
OVERRIDE int SDL_InitSubSystem(Uint32 flags);

/**
 *  This function cleans up all initialized subsystems. You should
 *  call it upon all exit conditions.
 */
OVERRIDE void SDL_Quit(void);

}

#endif
