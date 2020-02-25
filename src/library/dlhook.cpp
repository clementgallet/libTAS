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

#include "dlhook.h"
#include "logging.h"
#include "hook.h"
#include "wine/winehook.h"
#include "wine/wined3d.h"
#include <cstring>
#include <set>
#include "backtrace.h"

namespace libtas {

/* Set of libraries that are loaded by the game using dlopen */
/* It is stored static inside a function, so that it will be created on first
 * use. Indeed, it can be called very early in the program execution. If made
 * file-static, it may not be initialized in time.
 */

static std::set<std::string>& get_lib_set() {
    static std::set<std::string> library_set;
    return library_set;
}

std::string find_lib(const char* library)
{
    std::set<std::string>& library_set = get_lib_set();
    for (auto const& itr : library_set)
        if (itr.find(library) != std::string::npos)
            return itr;

    std::string emptystring;
    return emptystring;
}

void add_lib(const char* library)
{
    if (library) {
        std::set<std::string>& library_set = get_lib_set();
        library_set.insert(std::string(library));
    }
}

DEFINE_ORIG_POINTER(dlopen);
DEFINE_ORIG_POINTER(dlsym);

void *dlopen(const char *file, int mode) throw() {
    if (!orig::dlopen) {
        /* To access the real dlopen function, we use the fact that dlsym
         * calls internally _dl_sym.
         */
        orig::dlopen = reinterpret_cast<decltype(orig::dlopen)>(_dl_sym(RTLD_NEXT, "dlopen", reinterpret_cast<void*>(dlopen)));
    }

    if (GlobalState::isNative()) {
        return orig::dlopen(file, mode);
    }

    if (file != nullptr && std::string(file).find("libpulse") != std::string::npos) {
        debuglog(LCF_HOOK, __func__, " blocked access to library ", file);
        return nullptr;
    }

    debuglog(LCF_HOOK, __func__, " call with file ", (file!=nullptr)?file:"<NULL>");

    void *result = orig::dlopen(file, mode);

    if (result)
        add_lib(file);

    if (std::string(file).find("wined3d.dll.so") != std::string::npos) {
        /* Hook wine LdrGetProcedureAddress function */
        hook_wined3d();
    }

    return result;
}

void *find_sym(const char *name, bool original) {
    dlerror(); // Clear pending errors
    void *addr;
    NATIVECALL(addr = dlsym(RTLD_DEFAULT, name));
    if (dlerror() == nullptr) {
        Dl_info info;
        int res = dladdr(addr, &info);
        if (res != 0) {
            std::string libpath = info.dli_fname;
            std::string libtasstr;
            NATIVECALL(libtasstr = getenv("SDL_DYNAMIC_API"));
            bool fromLibtas = libpath.length() >= libtasstr.length() &&
                libpath.compare(libpath.length()-libtasstr.length(), libtasstr.length(), libtasstr) == 0;
            if (original == fromLibtas) {
                addr = nullptr;
            }
        }
    }
    return addr;
}

void *dlsym(void *handle, const char *name) throw() {
    if (!orig::dlsym) {
        /* Again, we use the internal `_dl_sym` function to access to the
         * location of the real `dlsym` function. This may seems weird, and is
         * also implementation-dependant, but it is simple. `_dl_sym` does not
         * have error-checking so we only use it here.
         */
        orig::dlsym = reinterpret_cast<decltype(orig::dlsym)>(_dl_sym(RTLD_NEXT, "dlsym", reinterpret_cast<void*>(dlsym)));
    }

    if (GlobalState::isNative()) {
        return orig::dlsym(handle, name);
    }

    debuglog(LCF_HOOK, __func__, " call with function ", name);

    /* Special cases when dlsym is called with dl* functions (yes, it happens...). */
    if (strcmp(name, "dlopen") == 0)
        return reinterpret_cast<void*>(dlopen);
    if (strcmp(name, "dlsym") == 0)
        return reinterpret_cast<void*>(dlsym);

    /* FIXME: This design is not good enough.
     * This idea is to link to our defined function when there is one, instead
     * of the function inside the library that the game wants to load.
     * However, there is a problem in this, encountered in Towerfall:
     * This game bundles with libpng 1.5.10, and load this library dynamically.
     * Also, the ffmpeg libraries that we use are usually compiled with png
     * support, which means that we statically link to our libpng (1.6.28 for me)
     * Thus, calling `dlsym(RTLD_DEFAULT, "png_xxx")` will return the function
     * from our installed libpng library, which has no reason to be of the same
     * version as the one bundled with the game.
     *
     * One solution would be to check if the symbol name is defined strictly
     * in our library, excluding any shared library dependency.
     * AFAIK, dlopen and dlsym always include all dependencies. Maybe by doing
     * some arithmetic pointer shenanigan?
     */

    void *addr = find_sym(name);
    if (addr == nullptr) {
        addr = orig::dlsym(handle, name);
    }

    if (0 == strcmp(name, "__wine_process_init")) {
        /* Hook wine LdrGetProcedureAddress function */
        hook_ntdll();
    }

    return addr;
}

}
