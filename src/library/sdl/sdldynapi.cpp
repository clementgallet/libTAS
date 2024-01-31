/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "general/dlhook.h"
#include "hook.h"
#include "GlobalState.h"
#include "../external/elfhacks.h"

#include <execinfo.h>
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

void setDynapiAddr(uint64_t addr)
{
    debuglogstdio(LCF_SDL, "Received SDL_DYNAPI_entry address %llx", addr);
    orig::SDL_DYNAPI_entry = reinterpret_cast<decltype(orig::SDL_DYNAPI_entry)>(addr);
}

/* Override */ Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize) {
    DEBUGLOGCALL(LCF_SDL);

    /* Try finding the original SDL_DYNAPI_entry function. The more generic way
     * is to determine in which file the calling function is located with  
     * dladdr(), and look into this file for the symbol `SDL_DYNAPI_entry`.
     *
     * For some reason, the code below does not find the symbol inside Unity
     * executables, even if bash command `readelf` shows the symbol...
     * So in this case, we received the symbol address from libtas program. */

    if (!orig::SDL_DYNAPI_entry) {
        Dl_info info;
        if (dladdr(__builtin_return_address(0), &info)) {
            /* Get the dynamic library name of our caller */

            debuglogstdio(LCF_SDL, "   Try extracting original SDL_DYNAPI_entry function from file %s", info.dli_fname);
            eh_obj_t obj;
            int ret = eh_find_obj(&obj, info.dli_fname);
            if (ret == 0) {
                eh_find_sym(&obj, "SDL_DYNAPI_entry", (void **) &orig::SDL_DYNAPI_entry);
                eh_destroy_obj(&obj);
            }
        }
    }

    if (!orig::SDL_DYNAPI_entry) {        
        /* We couldn't find the SDL library that is supposed to be used by the
        * game, so we load the system SDL library instead */
        debuglogstdio(LCF_SDL | LCF_WARNING, "   Could not find the original SDL_DYNAPI_entry function, using the system one");
        LINK_NAMESPACE_SDL2(SDL_DYNAPI_entry);
        
        if (!orig::SDL_DYNAPI_entry) {
            debuglogstdio(LCF_SDL | LCF_ERROR, "   Could not find any SDL_DYNAPI_entry function!");
            return 1;
        }
    }
    
    /* Get the original pointers. */
    Sint32 res = orig::SDL_DYNAPI_entry(apiver, table, tablesize);
    if (res != 0) {
        debuglogstdio(LCF_SDL | LCF_ERROR, "   The original SDL_DYNAPI_entry failed!");
        return res;
    }

    /* Now save original pointers while replacing them with our hooks. */
    void **entries = static_cast<void **>(table);

    /* TODO: Check SDL version, and try loading the system SDL instead of the
     * bundled SDL to get a version compatible with Dear ImGui. */
    // orig::SDL_GetVersion = reinterpret_cast<decltype(&SDL_GetVersion)>(entries[index::SDL_GetVersion]);
    // SDL_version ver = {0, 0, 0};
    // orig::SDL_GetVersion(&ver);
    // 
    // if ((ver.minor == 0) && (ver.patch < 18)) {
    //     debuglogstdio(LCF_SDL | LCF_WARNING, "System SDL library is too old (%d.%d.%d)!", ver.major, ver.minor, ver.patch);
    // }

    /* TODO: Load the `SDL_DYNAPI_entry()` function from system SDL library */
    
    // std::vector<void*> full_entries;
    // full_entries.resize(index::SDL_GetTouchName); // First function past 2.0.18
    // orig::SDL_DYNAPI_entry(apiver, full_entries.data(), full_entries.size() * sizeof(void *));
// #define IF_IN_BOUNDS_FULL(FUNC) if (index::FUNC < full_entries.size())

    char* libtaspath;
    NATIVECALL(libtaspath = getenv("SDL_DYNAMIC_API"));
    void *libtaslib;
    NATIVECALL(libtaslib = dlopen(libtaspath, RTLD_LAZY | RTLD_NOLOAD));
    if (libtaslib == nullptr) {
        debuglogstdio(LCF_SDL | LCF_ERROR, "   Could not find already loaded libtas.so!");
        return 1;
    }

#define IF_IN_BOUNDS(FUNC) if (index::FUNC * sizeof(void *) < tablesize)
#define SDL_LINK(FUNC) IF_IN_BOUNDS(FUNC) orig::FUNC = reinterpret_cast<decltype(&FUNC)>(entries[index::FUNC]); else debuglogstdio(LCF_SDL | LCF_HOOK, "sdl dynapi symbol %s will not be imported", #FUNC);
#define SDL_HOOK(FUNC) IF_IN_BOUNDS(FUNC) entries[index::FUNC] = reinterpret_cast<void *>(dlsym(libtaslib, #FUNC));
#include "sdlhooks.h"

    dlclose(libtaslib);
    return res;
}

}
