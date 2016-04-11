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

#include "hook.h"
#include "logging.h"
#include <string>
#include "dlhook.h"

bool link_function(void** function, const char* source, const char* library)
{
    /* Test if function is already linked */
    if (*function != nullptr)
        return true;

    dlenter(); // Use real dl functions

    /* First try to link it from the global namespace */
    *function = dlsym(RTLD_NEXT, source);

    if (*function != nullptr) {
        dlleave();
        return true;
    }

    /* If it did not succeed, try to link using a matching library
     * loaded by the game.
     */

    if (library != nullptr) {
        std::string libpath = find_lib(library);

        if (! libpath.empty()) {

            /* Try to link again using a matching library */
            void* handle = dlopen(libpath.c_str(), RTLD_LAZY);

            if (handle != NULL) {
                *function = dlsym(handle, source);

                if (*function != nullptr) {
                    dlleave();
                    return true;
                }
            }
        }
    }
    debuglog(LCF_ERROR | LCF_HOOK, "Could not import symbol ", source);

    *function = nullptr;
    dlleave();
    return false;
}

int SDLver = 0;

void (*SDL_GetVersion_real)(SDL_version* ver);
/* SDL 1.2 specific functions */
SDL_version * (*SDL_Linked_Version_real)(void);

int get_sdlversion(void)
{
    if (SDLver != 0)
        return 1;

    LINK_SUFFIX(SDL_GetVersion, "libSDL2-2");
    if (SDL_GetVersion_real == nullptr)
        LINK_SUFFIX(SDL_Linked_Version, "libSDL-1.2");

    /* Determine SDL version */
    SDL_version ver = {0, 0, 0};
    if (SDL_GetVersion_real) {
        SDL_GetVersion_real(&ver);
    }
    else if (SDL_Linked_Version_real) {
        SDL_version *verp;
        verp = SDL_Linked_Version_real();
        ver = *verp;
    }

    debuglog(LCF_SDL | LCF_HOOK, "Detected SDL ", (int)ver.major, ".", (int)ver.minor, ".", (int)ver.patch);

    /* We save the version major in an extern variable because we will use it elsewhere */
    SDLver = ver.major;

    if (ver.major == 0) {
        debuglog(LCF_ERROR | LCF_SDL | LCF_HOOK, "Could not get SDL version...");
        return 0;
    }

    return 1;
}

