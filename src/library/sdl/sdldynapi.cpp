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

#include "sdldynapi.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"
#include "../external/elfhacks.h"

#include <execinfo.h>
#include <dlfcn.h>

namespace libtas {

static Uint32 sdl_apiver = 0;
static void** orig_sdl_table = nullptr;

int getSDLApiver()
{
    return sdl_apiver;
}

void** getOrigSDLFuncLoc(int index_sdl)
{
    return getOrigSDLFuncLoc(index_sdl, index_sdl);
}

void** getOrigSDLFuncLoc(int index_sdl2, int index_sdl3)
{
    if (sdl_apiver == 1) {
        return &orig_sdl_table[index_sdl2];
    }
    else if (sdl_apiver == 2) {
        return &orig_sdl_table[index_sdl3];
    }
    else {
        LOG(LL_ERROR, LCF_SDL, "SDL API version is not set!");
        return nullptr;
    }
}

decltype(&SDL_DYNAPI_entry) orig_SDL_DYNAPI_entry;

void setDynapiAddr(uint64_t addr)
{
    LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "Received SDL_DYNAPI_entry address %llx", addr);
    orig_SDL_DYNAPI_entry = reinterpret_cast<decltype(&SDL_DYNAPI_entry)>(addr);
}

/* Override */ Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize) {
    LOGTRACE(LCF_SDL | LCF_HOOK);

    sdl_apiver = apiver;
    if ((apiver != 1) && (apiver != 2)) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "Unsupported SDL API version %d!", apiver);
        return 1;
    }

    /* Try finding the original SDL_DYNAPI_entry function. The more generic way
     * is to determine in which file the calling function is located with  
     * dladdr(), and look into this file for the symbol `SDL_DYNAPI_entry`.
     *
     * For some reason, the code below does not find the symbol inside Unity
     * executables, even if bash command `readelf` shows the symbol...
     * So in this case, we received the symbol address from libtas program. */

    if (!orig_SDL_DYNAPI_entry) {
        Dl_info info;
        if (dladdr(__builtin_return_address(0), &info)) {
            /* Get the dynamic library name of our caller */

            LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   Try extracting original SDL_DYNAPI_entry function from file %s", info.dli_fname);
            eh_obj_t obj;
            int ret = eh_find_obj(&obj, info.dli_fname);
            if (ret == 0) {
                eh_find_sym(&obj, "SDL_DYNAPI_entry", (void **) &orig_SDL_DYNAPI_entry);
                eh_destroy_obj(&obj);
            }
        }
    }

    if (!orig_SDL_DYNAPI_entry) {
        /* We couldn't find the SDL library that is supposed to be used by the
        * game, so we load the system SDL library instead */
        LOG(LL_WARN, LCF_SDL | LCF_HOOK, "   Could not find the original SDL_DYNAPI_entry function, using the system one");
        if (apiver == 1)
            link_function((void**)&orig_SDL_DYNAPI_entry, "SDL_DYNAPI_entry", "libSDL2-2.0.so.0");
        else if (apiver == 2)
            link_function((void**)&orig_SDL_DYNAPI_entry, "SDL_DYNAPI_entry", "libSDL3.so.0");
        
        if (!orig_SDL_DYNAPI_entry) {
            LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "   Could not find any SDL_DYNAPI_entry function!");
            return 1;
        }
    }
    
    /* Get the original pointers. */
    Sint32 res = orig_SDL_DYNAPI_entry(apiver, table, tablesize);
    if (res != 0) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "   The original SDL_DYNAPI_entry failed!");
        return res;
    }
    else {
        LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   The original SDL_DYNAPI_entry succeeded with %d functions", tablesize / sizeof(void *));
    }

    /* Now save original pointers while replacing them with our hooks. */
    void **entries = static_cast<void **>(table);

    orig_sdl_table = reinterpret_cast<void**>(malloc(tablesize));
    memcpy(orig_sdl_table, table, tablesize);

    /* TODO: Check SDL version, and try loading the system SDL instead of the
     * bundled SDL to get a version compatible with Dear ImGui. */
    // orig::SDL_GetVersion = reinterpret_cast<decltype(&SDL_GetVersion)>(entries[index_sdl2::SDL_GetVersion]);
    // SDL_version ver = {0, 0, 0};
    // orig::SDL_GetVersion(&ver);
    // 
    // if ((ver.minor == 0) && (ver.patch < 18)) {
    //     LOG(LL_WARN, LCF_SDL, "System SDL library is too old (%d.%d.%d)!", ver.major, ver.minor, ver.patch);
    // }

    /* TODO: Load the `SDL_DYNAPI_entry()` function from system SDL library */
    
    // std::vector<void*> full_entries;
    // full_entries.resize(index_sdl2::SDL_GetTouchName); // First function past 2.0.18
    // orig::SDL_DYNAPI_entry(apiver, full_entries.data(), full_entries.size() * sizeof(void *));
// #define IF_IN_BOUNDS_FULL(FUNC) if (index_sdl2::FUNC < full_entries.size())

    char* libtaspath;
    NATIVECALL(libtaspath = getenv("SDL_DYNAMIC_API"));
    if (libtaspath == nullptr) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "   SDL_DYNAMIC_API is not set, cannot resolve libtas SDL dynapi hooks");
        return 1;
    }

    void *libtaslib;
    NATIVECALL(libtaslib = dlopen(libtaspath, RTLD_LAZY | RTLD_NOLOAD));
    if (libtaslib == nullptr) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "   Could not find already loaded libtas.so!");
        return 1;
    }

    /* For each SDL function that we define, we assign its function pointer into the table
     * returned by the original SDL_DYNAPI_entry function. First we look if the function
     * defined inside its namespace (sdl2 or sdl3) has an implementation (it was defined
     * with a weak symbol). If it does not exist, we try to load it dynamically from our
     * own library with the symbol name.
     * The former method is necessary when the same function exists in both SDL2 and SDL3, but
     * with a different signature. The latter method is still present because wheneter possible,
     * we left SDL functions as visible symbols, to support the standard way of hooking (LD_PRELOAD).
     * This is still necessary for old games that were bundled with SDL1, or old SDL2 without
     * dynapi support, or for games that deliberately disabled it. */

    if (apiver == 1) {
        LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   Hooking SDL 2 functions...");

#define IF_IN_BOUNDS(FUNC) if (index_sdl2::FUNC * sizeof(void *) < tablesize)
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) \
    IF_IN_BOUNDS(fn) { \
        /* We first try to hook with a function defined in the sdl2 namespace. \
         * The sdl2 namespace symbols were declared weak, so we can check if they are defined or not. \
         * This is needed when the same function exists in both SDL2 and SDL3, but \
         * with a different signature. */ \
        void *sym = reinterpret_cast<void*>(&sdl2::fn); \
        if (sym) { \
            LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   %s -> %p", #fn, sym); \
            entries[index_sdl2::fn] = sym; \
        } \
        else { \
            GlobalNative gn; \
            sym = dlsym(libtaslib, #fn); \
            if (sym) { \
            LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   %s -> %p", #fn, sym); \
                entries[index_sdl2::fn] = sym; \
            } \
        } \
    }
#define SDL_DYNAPI_PROC_NO_VARARGS 1
#include "../../external/SDL2_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
#undef IF_IN_BOUNDS
    }
    else if (apiver == 2) {
        LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   Hooking SDL 3 functions...");

#define IF_IN_BOUNDS(FUNC) if (index_sdl3::FUNC * sizeof(void *) < tablesize)
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) \
    IF_IN_BOUNDS(fn) { \
        /* We first try to hook with a function defined in the sdl3 namespace. \
         * The sdl3 namespace symbols were declared weak, so we can check if they are defined or not. \
         * This is needed when the same function exists in both SDL2 and SDL3, but \
         * with a different signature. */ \
        void *sym = reinterpret_cast<void*>(&sdl3::fn); \
        if (sym) { \
            LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   %s -> %p", #fn, sym); \
            entries[index_sdl3::fn] = sym; \
        } \
        else { \
            GlobalNative gn; \
            sym = dlsym(libtaslib, #fn); \
            if (sym) { \
                LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "   %s -> %p", #fn, sym); \
                entries[index_sdl3::fn] = sym; \
            } \
        } \
    }
#define SDL_DYNAPI_PROC_NO_VARARGS 1
#include "../../external/SDL3_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
#undef IF_IN_BOUNDS
    }

    dlclose(libtaslib);
    return res;
}

}
