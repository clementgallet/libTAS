/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdldynapi.h"

#include "logging.h"
#include "hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(SDL_DYNAPI_entry);
#define SDL_LINK(FUNC) DEFINE_ORIG_POINTER(FUNC);
#define SDL_HOOK(FUNC) SDL_LINK(FUNC)
#include "sdlhooks.h"

namespace index {
enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
#define SDL_DYNAPI_PROC_NO_VARARGS 0
#include "../external/SDL_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
};
}

/* Override */ Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize) {
    DEBUGLOGCALL(LCF_SDL);

    /* We cannot call any SDL functions until dynapi is setup, including the
     * get_sdlversion in LINK_NAMESPACE_SDLX.  However, dynapi was not
     * introduced until 2.0.2, so we can assume SDL2 for now.
     */
    LINK_NAMESPACE_SDL2(SDL_DYNAPI_entry);

    /* Get the original pointers. */
    Sint32 res = orig::SDL_DYNAPI_entry(apiver, table, tablesize);
    if (res != 0) {
        debuglog(LCF_SDL | LCF_ERROR, "The original SDL_DYNAPI_entry failed!");
        return res;
    }

    /* Now save original pointers while replacing them with our hooks. */
    void **entries = static_cast<void **>(table);
#define SDL_LINK(FUNC) orig::FUNC = reinterpret_cast<decltype(&FUNC)>(entries[index::FUNC]);
#define SDL_HOOK(FUNC) SDL_LINK(FUNC); entries[index::FUNC] = reinterpret_cast<void *>(FUNC);
#include "sdlhooks.h"

    return res;
}

}
