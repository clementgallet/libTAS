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

/* Define some SDL functions that appear in version 2.0.6, because still many
 * distributions are bundled with version 2.0.5 */
#include "inputs/sdljoystick.h"

#include "logging.h"
#include "dlhook.h"
#include "hook.h"

#include <dlfcn.h>

namespace libtas {

DEFINE_ORIG_POINTER(SDL_DYNAPI_entry);
#define SDL_LINK(FUNC) DEFINE_ORIG_POINTER(FUNC);
#define SDL_HOOK(FUNC)
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

    /* First try to use functions from the main executable in case it is
     * statically linked with a modified version of SDL.  If this fails, the
     * LINK_NAMESPACE below will use the dynamic library instead. */
    orig::SDL_DYNAPI_entry = reinterpret_cast<decltype(orig::SDL_DYNAPI_entry)>(find_sym("SDL_DYNAPI_entry", true));

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
    void *libtas = dlopen("libtas.so", RTLD_LAZY | RTLD_NOLOAD);
    if (libtas == nullptr) {
        debuglog(LCF_SDL | LCF_ERROR, "Could not find already loaded libtas.so!");
        return 1;
    }

    void **entries = static_cast<void **>(table);
#define IF_IN_BOUNDS(FUNC) if (index::FUNC * sizeof(void *) < tablesize)
#define SDL_LINK(FUNC) IF_IN_BOUNDS(FUNC) orig::FUNC = reinterpret_cast<decltype(&FUNC)>(entries[index::FUNC]); else debuglog(LCF_ERROR | LCF_SDL | LCF_HOOK, "Could not import sdl dynapi symbol ", #FUNC);
#define SDL_HOOK(FUNC) IF_IN_BOUNDS(FUNC) entries[index::FUNC] = reinterpret_cast<void *>(dlsym(libtas, #FUNC));
#include "sdlhooks.h"

    dlclose(libtas);
    return res;
}

}
