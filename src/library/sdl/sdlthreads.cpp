/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "logging.h"
#include "hook.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateThread)
DECLARE_ORIG_POINTER(SDL_WaitThread)

/* Override */ SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    if (get_sdlversion() == 1)
        /* SDL1 version prototype is SDL_CreateThread(int (SDLCALL *fn)(void *), void *data)
         * So we can't print the thread name */
        LOGTRACE(LCF_THREAD);
    else
        LOG(LL_TRACE, LCF_THREAD, "SDL Thread %s was created.", name);
    LINK_NAMESPACE_SDLX(SDL_CreateThread);
    return orig::SDL_CreateThread(fn, name, data);
}

/* Override */ void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    LOGTRACE(LCF_THREAD);
    LINK_NAMESPACE_SDLX(SDL_WaitThread);
    orig::SDL_WaitThread(thread, status);
}

}
