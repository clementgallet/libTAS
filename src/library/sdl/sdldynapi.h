/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLDYNAPI_H_INCLUDED
#define LIBTAS_SDLDYNAPI_H_INCLUDED

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include "sdl/sdlversion.h"

#include <vector>

namespace libtas {

void setDynapiAddr(uint64_t addr);
void setSDLFullscreenAddr(uint64_t addr);

namespace index_sdl2 {
    enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
#define SDL_DYNAPI_PROC_NO_VARARGS 0
#include "../../external/SDL2_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
        SDL_EnumCount
    };
}

namespace index_sdl3 {
    enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
// As opposed to SDL2, in SDL3, we need SDL_DYNAPI_PROC_NO_VARARGS to be not defined!
#include "../../external/SDL3_dynapi_procs.h"
#undef SDL_DYNAPI_PROC
        SDL_EnumCount
    };
}

int getSDLApiver();

/* Return a pointer to the location of the original SDL2 or SDL3 function, based on its index. */
void** getOrigSDLFuncLoc(int index_sdl);
void** getOrigSDLFuncLoc(int index_sdl2, int index_sdl3);

#ifdef __unix__
#define LINK_NAMESPACE_SDL1(FUNC) link_function((void**)&orig::FUNC, #FUNC, "libSDL-1.2.so.0")
#elif defined(__APPLE__) && defined(__MACH__)
#define LINK_NAMESPACE_SDL1(FUNC) link_function((void**)&orig::FUNC, #FUNC, "libSDL-1.2.0.dylib")
#endif

/* Return a pointer to the original SDL2 or SDL3 function */
#define ORIG_SDL2_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl2::FUNC)
#define ORIG_SDL3_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl3::FUNC)

/* When both SDL2 and SDL3 functions are available, return either one */
#define ORIG_SDL23_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl2::FUNC, index_sdl3::FUNC)

/* Call a SDL2 function, and optionally link it if it is not defined already.
 * We also catch here SDL1 functions that are available in SDL2. */
#define ORIG_SDL2_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl2::FUNC), #FUNC, get_sdlversion()==1?"libSDL-1.2.so.0":"libSDL2-2.0.so.0")
#define ORIG_SDL2_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl2::FUNC)>(ORIG_SDL2_LINK(FUNC)) PARAMS

/* Call a SDL3 function */
#define ORIG_SDL3_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl3::FUNC), #FUNC, "libSDL3.so.0")
#define ORIG_SDL3_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl3::FUNC)>(ORIG_SDL3_LINK(FUNC)) PARAMS

/* Call a SDL2 or SDL3 function depending on which version is used. The function signature must be the same!
 * We also catch here SDL1 functions that are available in SDL2/SDL3. */
#define ORIG_SDL23_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl2::FUNC, index_sdl3::FUNC), #FUNC, get_sdlversion()==1?"libSDL-1.2.so.0":(get_sdlversion()==2?"libSDL2-2.0.so.0":"libSDL3.so.0"))
#define ORIG_SDL23_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl2::FUNC)>(ORIG_SDL23_LINK(FUNC)) PARAMS

/**
 * This function initializes the SDL jump table.
 */
OVERRIDE Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize);

}

#endif
