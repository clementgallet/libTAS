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

#include "sdlthreads.h"
#include "sdldynapi.h"

#include "logging.h"

namespace libtas {

/* Override */ sdl2::SDL_Thread* SDL_CreateThread(sdl2::SDL_ThreadFunction fn, const char *name, void *data)
{
    if (get_sdlversion() == 1)
        /* SDL1 version prototype is SDL_CreateThread(int (SDLCALL *fn)(void *), void *data)
         * So we can't print the thread name */
        LOGTRACE(LCF_THREAD);
    else
        LOG(LL_TRACE, LCF_THREAD, "SDL Thread %s was created.", name);
    return ORIG_SDL2_CALL(SDL_CreateThread, (fn, name, data));
}

/* Override */ void SDL_WaitThread(sdl2::SDL_Thread * thread, int *status)
{
    LOGTRACE(LCF_THREAD);
    return ORIG_SDL2_CALL(SDL_WaitThread, (thread, status));
}

}
