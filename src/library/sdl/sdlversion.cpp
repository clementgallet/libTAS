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

#include "sdlversion.h"
#include "sdldynapi.h"

#include "hook.h"
#include "logging.h"
#include "GlobalState.h"

#include <dlfcn.h>
#include "../external/SDL1.h" // SDL_version
#include "../external/SDL2.h" // SDL_version

namespace libtas {

namespace orig {
    /* SDL 1.2 specific function */
    static SDL_version * (*SDL_Linked_Version)(void);
    static void (*SDL2_GetVersion)(SDL_version * ver);
    static int (*SDL3_GetVersion)();
}

int get_sdlversion(void)
{
    /* We save the version major so that we can return it in a future calls */
    static int SDLver = -1;

    if (SDLver != -1)
        return SDLver;

    /* Determine SDL version. SDL1 and SDL2 versions have the same struct, so using either one. */
    SDL_version ver = {0, 0, 0};

    /* First look if symbols are already accessible */
    void* SDL_GetVersion_ptr = *ORIG_SDL23_FUNCTION_POINTER(SDL_GetVersion);

    if (SDL_GetVersion_ptr) {
        /* Both SDL2 and SDL3 have the same function name with a different signature.
         * To differentiate, we check the SDL API version. */
        if (getSDLApiver() == 2) {
            orig::SDL3_GetVersion = reinterpret_cast<decltype(orig::SDL3_GetVersion)>(SDL_GetVersion_ptr);
        }
        else {
            orig::SDL2_GetVersion = reinterpret_cast<decltype(orig::SDL2_GetVersion)>(SDL_GetVersion_ptr);
        }
    }
    if (!SDL_GetVersion_ptr) {
        NATIVECALL(orig::SDL_Linked_Version = (decltype(orig::SDL_Linked_Version)) dlsym(RTLD_DEFAULT, "SDL_Linked_Version"));
        if (!orig::SDL_Linked_Version) {
            /* We rely on the fact that it is more likely for SDL2 to miss the dynapi feature than SDL3,
             * so we check for SDL2 as a fallback. */
            NATIVECALL(orig::SDL2_GetVersion = reinterpret_cast<decltype(orig::SDL2_GetVersion)>(dlsym(RTLD_DEFAULT, "SDL_GetVersion")));
        }
    }

    if (orig::SDL3_GetVersion) {
        int ver_int = orig::SDL3_GetVersion();
        ver.major = SDL_VERSIONNUM_MAJOR(ver_int);
        ver.minor = SDL_VERSIONNUM_MINOR(ver_int);
        ver.patch = SDL_VERSIONNUM_MICRO(ver_int);
    }

    if (orig::SDL2_GetVersion) {
        orig::SDL2_GetVersion(&ver);
    }

    if (orig::SDL_Linked_Version) {
        SDL_version *verp = orig::SDL_Linked_Version();
        ver = *verp;
    }

    if (orig::SDL2_GetVersion && orig::SDL_Linked_Version) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "Both SDL versions were detected! Taking SDL1 in priority");
    }

    if (ver.major > 0) {
        SDLver = ver.major;
        LOG(LL_DEBUG, LCF_SDL | LCF_HOOK, "Detected SDL %d.%d.%d", static_cast<int>(ver.major), static_cast<int>(ver.minor), static_cast<int>(ver.patch));
        return SDLver;
    }

    /* If not, determine which library was already dynamically loaded. */
    void *sdl1_handle, *sdl2_handle, *sdl3_handle;
#ifdef __unix__
    NATIVECALL(sdl1_handle = dlopen("libSDL-1.2.so.0", RTLD_NOLOAD));
    NATIVECALL(sdl2_handle = dlopen("libSDL2-2.0.so.0", RTLD_NOLOAD));
    NATIVECALL(sdl3_handle = dlopen("libSDL3.so.0", RTLD_NOLOAD));
#elif defined(__APPLE__) && defined(__MACH__)
    NATIVECALL(sdl1_handle = dlopen("libSDL-1.2.0.dylib", RTLD_NOLOAD));
    NATIVECALL(sdl2_handle = dlopen("libSDL2-2.0.0.dylib", RTLD_NOLOAD));
    NATIVECALL(sdl3_handle = dlopen("libSDL3.so.0", RTLD_NOLOAD));
#endif

    if (sdl1_handle && !sdl2_handle && !sdl3_handle) {
        SDLver = 1;
    }
    else if (!sdl1_handle && sdl2_handle && !sdl3_handle) {
        SDLver = 2;
    }
    else if (!sdl1_handle && !sdl2_handle && sdl3_handle) {
        SDLver = 3;
    }
    else if (!sdl1_handle && !sdl2_handle && !sdl3_handle) {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "No SDL versions were detected!");
    }
    else {
        LOG(LL_ERROR, LCF_SDL | LCF_HOOK, "Multiple SDL versions were detected! Taking the lowest available version");
        if (sdl1_handle) SDLver = 1;
        else SDLver = 2;
    }

    if (sdl1_handle) dlclose(sdl1_handle);
    if (sdl2_handle) dlclose(sdl2_handle);
    if (sdl3_handle) dlclose(sdl3_handle);

    return SDLver;
}

}
