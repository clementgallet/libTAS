/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "../inputs/sdljoystick.h"

#include "../logging.h"
#include "../dlhook.h"
#include "../hook.h"

#include <dlfcn.h>

namespace libtas {

DEFINE_ORIG_POINTER(SDL_DYNAPI_entry)
#define SDL_LINK(FUNC) DEFINE_ORIG_POINTER(FUNC)
#define SDL_HOOK(FUNC)
#include "sdlhooks.h"

namespace index {
enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
#define SDL_DYNAPI_PROC_NO_VARARGS 0
#include "../../external/SDL_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
};
}

/* Override */ Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize) {
    DEBUGLOGCALL(LCF_SDL);

    /* First try to use functions from the main executable in case it is
     * statically linked with a modified version of SDL.  If this fails, the
     * LINK_NAMESPACE below will use the dynamic library instead. */
    void* h;
#ifdef __unix__
    NATIVECALL(h = dlopen(nullptr, RTLD_DEEPBIND));
#elif defined(__APPLE__) && defined(__MACH__)
    NATIVECALL(h = dlopen(nullptr, RTLD_FIRST));
#endif
    NATIVECALL(orig::SDL_DYNAPI_entry = reinterpret_cast<decltype(orig::SDL_DYNAPI_entry)>(dlsym(h, "SDL_DYNAPI_entry")));

    /* This looks weird to use dlopen/dlsym to find the current function pointer,
     * but in games that bundle their own libSDL (e.g. Iconoclasts), even using
     * pointer `libtas::SDL_DYNAPI_entry` does not refer to this function but to 
     * the game's one. */
     
    char* libtaspath;
    NATIVECALL(libtaspath = getenv("SDL_DYNAMIC_API"));
    void *libtas;
    NATIVECALL(libtas = dlopen(libtaspath, RTLD_LAZY | RTLD_NOLOAD));
    if (libtas == nullptr) {
        debuglogstdio(LCF_SDL | LCF_ERROR, "Could not find already loaded libtas.so!");
        return 1;
    }

    void *current_func;
    NATIVECALL(current_func = dlsym(libtas, "SDL_DYNAPI_entry"));
    if (current_func == nullptr) {
        debuglogstdio(LCF_SDL | LCF_ERROR, "Could not find own SDL_DYNAPI_entry function!");
        return 1;
    }

    /* Check if the function pointer we found is this function. In that case,
     * invalide the pointer, so we use the second method. */
    if (reinterpret_cast<void*>(orig::SDL_DYNAPI_entry) == current_func) {
        orig::SDL_DYNAPI_entry = nullptr;
    }

    /* We cannot call any SDL functions until dynapi is setup, including the
     * get_sdlversion in LINK_NAMESPACE_SDLX.  However, dynapi was not
     * introduced until 2.0.2, so we can assume SDL2 for now.
     * 
     * We don't use the full library name first, because it is supposed to be already
     * accessible. Some games or frameworks don't use the exact library name.
     */
    LINK_NAMESPACE_FULLNAME(SDL_DYNAPI_entry, "libSDL2");

    if (!orig::SDL_DYNAPI_entry) {
        
        /* We couldn't find the SDL library that is supposed to be used by the
         * game, so we load the system SDL library instead */
        debuglogstdio(LCF_SDL | LCF_WARNING, "Could not find the original SDL_DYNAPI_entry function, using the system one");
        LINK_NAMESPACE_SDL2(SDL_DYNAPI_entry);

        if (!orig::SDL_DYNAPI_entry) {
            debuglogstdio(LCF_SDL | LCF_ERROR, "Could not find any SDL_DYNAPI_entry function!");
            return 1;
        }
    }
    
    /* Get the original pointers. */
    Sint32 res = orig::SDL_DYNAPI_entry(apiver, table, tablesize);
    if (res != 0) {
        debuglogstdio(LCF_SDL | LCF_ERROR, "The original SDL_DYNAPI_entry failed!");
        return res;
    }

    /* Now save original pointers while replacing them with our hooks. */
    void **entries = static_cast<void **>(table);
#define IF_IN_BOUNDS(FUNC) if (index::FUNC * sizeof(void *) < tablesize)
#define SDL_LINK(FUNC) IF_IN_BOUNDS(FUNC) orig::FUNC = reinterpret_cast<decltype(&FUNC)>(entries[index::FUNC]); else debuglogstdio(LCF_ERROR | LCF_SDL | LCF_HOOK, "Could not import sdl dynapi symbol %s", #FUNC);
#define SDL_HOOK(FUNC) IF_IN_BOUNDS(FUNC) entries[index::FUNC] = reinterpret_cast<void *>(dlsym(libtas, #FUNC));
#include "sdlhooks.h"

    dlclose(libtas);
    return res;
}

}
