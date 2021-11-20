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
#ifdef __unix__
#include "wine/winehook.h"
#include "wine/wined3d.h"
#include "wine/user32.h"
#include "wine/kernel32.h"
#include "../external/elfhacks.h"
#endif
#include <cstring>
#include <set>
#include "backtrace.h"
#include "GameHacks.h"
#include <sys/stat.h>
#include "../dyld_func_lookup_helper/dyld_func_lookup_helper.h"

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

DEFINE_ORIG_POINTER(dlopen)
DEFINE_ORIG_POINTER(dlsym)

/* Find the address of dlopen and dlsym symbols */
static void get_dlfct_symbols()
{
#ifdef __unix__
    /* To access the real dl functions, we use the fact that dlsym
     * calls internally _dl_sym. */
    if (_dl_sym) {
        orig::dlopen = reinterpret_cast<decltype(orig::dlopen)>(_dl_sym(RTLD_NEXT, "dlopen", reinterpret_cast<void*>(dlopen)));
        orig::dlsym = reinterpret_cast<decltype(orig::dlsym)>(_dl_sym(RTLD_NEXT, "dlsym", reinterpret_cast<void*>(dlsym)));
    }
    else {
        /* If _dl_sym is not available (such as with glibc >= 2.34), find the
         * symbols manually by parsing the loaded library */
        /* Code taken from MangoHud: <https://github.com/flightlessmango/MangoHud> */
        const char* libs[] = {
#if defined(__GLIBC__)
            "*libdl.so*",
#endif
            "*libc.so*",
            "*libc.*.so*",
        };

        for (size_t i = 0; i < sizeof(libs) / sizeof(*libs); i++)
        {
            eh_obj_t libdl;
            int ret = eh_find_obj(&libdl, libs[i]);
            if (ret)
                continue;

            eh_find_sym(&libdl, "dlopen", (void **) &orig::dlopen);
            eh_find_sym(&libdl, "dlsym", (void **) &orig::dlsym);
            eh_destroy_obj(&libdl);

            if (orig::dlopen && orig::dlsym)
                break;
            orig::dlopen = nullptr;
            orig::dlsym = nullptr;
        }
    }
#elif defined(__APPLE__) && defined(__MACH__)
    /* Using the convenient function to locate a dyld function pointer */
    dyld_func_lookup_helper("__dyld_dlopen", reinterpret_cast<void**>(&orig::dlopen));
    dyld_func_lookup_helper("__dyld_dlsym", reinterpret_cast<void**>(&orig::dlsym));
#endif

    if (!orig::dlopen && !orig::dlsym)
    {
        debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not get dl function symbols");
        exit(1);
    }
}

#if defined(__APPLE__) && defined(__MACH__)
/* dlopen_from is like dlopen, except it takes an extra argument specifying the "true" caller address.
 * it is marked as weak so that libTAS will still work if it's not present. */
extern "C" void *dlopen_from(const char *file, int mode, void *callerAddr) __attribute__((weak));
#endif

__attribute__((noipa)) void *dlopen(const char *file, int mode) __THROW {
    void *const callerAddr = __builtin_extract_return_addr(__builtin_return_address(0));

    if (!orig::dlopen) {
        get_dlfct_symbols();
    }

    if (GlobalState::isNative()) {
        return orig::dlopen(file, mode);
    }

    if (file != nullptr && std::strstr(file, "libpulse") != nullptr) {
        debuglogstdio(LCF_HOOK, "%s blocked access to library %s", __func__, file);
        return nullptr;
    }

    if (file != nullptr && std::strstr(file, "ScreenSelector.so") != nullptr) {
        debuglogstdio(LCF_HOOK, "%s blocked access to library %s", __func__, file);
        return nullptr;
    }

    debuglogstdio(LCF_HOOK, "%s call with file %s", __func__, (file!=nullptr)?file:"<NULL>");
    void *result = nullptr;

#if defined(__APPLE__) && defined(__MACH__)
    if (dlopen_from) {
        result = dlopen_from(file, mode, reinterpret_cast<void*>(callerAddr));

        if (result != nullptr) {
            add_lib(file);
        }
    } else
#endif
    {
#ifdef __unix__
        if (file != nullptr && file[0] != '\0' && std::strchr(file, '/') == nullptr) {
            /* Path should be searched using search paths, so let's
             * manually check the paths in the correct order...
             */
            Dl_info info;
            if (dladdr(callerAddr, &info)) {
                /* Get the dynamic library name of our caller */
                const char *dlname = info.dli_fname;
                {
                    struct stat dlstat, exestat;
                    if (stat(dlname, &dlstat) == 0 &&
                        stat("/proc/self/exe", &exestat) == 0 &&
                        dlstat.st_dev == exestat.st_dev &&
                        dlstat.st_ino == exestat.st_ino) {
                        /* Unless being called from the main executable */
                        dlname = nullptr;
                    }
                }

                /* Open object of caller */
                void *caller = orig::dlopen(dlname, RTLD_LAZY | RTLD_NOLOAD);
                if (caller != nullptr) {
                    Dl_serinfo size;
                    if (dlinfo(caller, RTLD_DI_SERINFOSIZE, &size) == 0) {
                        auto *paths = reinterpret_cast<Dl_serinfo *>(new char[size.dls_size]);
                        *paths = size;

                        /* Get ordered list of search paths for this object */
                        if (dlinfo(caller, RTLD_DI_SERINFO, paths) == 0) {
                            for (unsigned i = 0; i != paths->dls_cnt; ++i) {
                                const char *name = paths->dls_serpath[i].dls_name;
                                /* Probably can't happen, just being safe... */
                                if (name == nullptr || name[0] == '\0')
                                    continue;
                                std::string path(name);
                                /* Note that this guaranteed / prevents
                                 * recursive search path lookup
                                 */
                                if (path.back() != '/')
                                    path += '/';
                                path += file;

                                result = orig::dlopen(path.c_str(), mode);
                                if (result != nullptr) {
                                    debuglogstdio(LCF_HOOK, "   Found at %s", name);
                                    add_lib(path.c_str());
                                    break;
                                }
                            }
                        }

                        delete [] reinterpret_cast<char *>(paths);
                    }

                    dlclose(caller);
                }
            }
        }
#endif

        if (result == nullptr) {
            /* Path is empty (referring to the main program), relative to
             * the cwd, absolute, or failed search path lookup above, so
             * try looking it up normally.
             */
            result = orig::dlopen(file, mode);

            if (result != nullptr)
                add_lib(file);
        }
    }

#ifdef __linux__
    if (result && file && std::strstr(file, "wined3d.dll.so") != nullptr) {
        /* Hook wine wined3d functions */
        hook_wined3d();
    }

    if (result && file && std::strstr(file, "user32.dll.so") != nullptr) {
        /* Hook wine user32 functions */
        hook_user32();
    }

    if (result && file && std::strstr(file, "kernel32.dll.so") != nullptr) {
        /* Hook wine kernel32 functions */
        hook_kernel32();
    }
#endif

    if (result && file && std::strstr(file, "libcoreclr.so") != nullptr)
        GameHacks::setCoreclr();

    return result;
}

void *find_sym(const char *name, bool original) {
    dlerror(); // Clear pending errors
    void *addr = orig::dlsym(RTLD_DEFAULT, name);
    if (dlerror() == nullptr) {
        Dl_info info;
        int res = dladdr(addr, &info);
        if (res != 0) {
            std::string libpath = info.dli_fname;
            std::string libtasstr;
            NATIVECALL(libtasstr = getenv("LIBTAS_LIBRARY_PATH"));
            bool fromLibtas = libpath.length() >= libtasstr.length() &&
                libpath.compare(libpath.length()-libtasstr.length(), libtasstr.length(), libtasstr) == 0;
            if (original == fromLibtas) {
                addr = nullptr;
            }
        }
    }
    return addr;
}

void *dlsym(void *handle, const char *name) __THROW {
    if (!orig::dlsym) {
        get_dlfct_symbols();
    }

    /* dlsym() does some work besides the actual function, and that may call
     * other hooked functions that themselves call dlsym(). If we detect a recursion,
     * we switch to the pure dlsym() function like above.
     * This is especially the case for jemalloc which calls a bunch of functions,
     * and dlsym allocates buffers for dlerror().
     */
    static int recurs_count = 0;
    static bool safe = false;
    
    safe = (recurs_count > 0);
    recurs_count++;
    
    if (GlobalState::isNative()) {
        void* ret;
        if (safe) {
#ifdef __unix__
            ret = _dl_sym(handle, name, reinterpret_cast<void*>(dlsym));
#elif defined(__APPLE__) && defined(__MACH__)
            /* TODO */
            ret = orig::dlsym(handle, name);
#endif
        }
        else {
            ret = orig::dlsym(handle, name);            
        }
        recurs_count--;
        return ret;
    }

    debuglogstdio(LCF_HOOK, "%s call with function %s %s", __func__, name, safe?"(safe)":"");

    /* Special cases when dlsym is called with dl* functions (yes, it happens...). */
    if (strcmp(name, "dlopen") == 0) {
        recurs_count--;
        return reinterpret_cast<void*>(dlopen);
    }
    if (strcmp(name, "dlsym") == 0) {
        recurs_count--;        
        return reinterpret_cast<void*>(dlsym);
    }

    /* Special case for RTLD_NEXT. The order of loaded libraries are affected
     * by us preloading libtas.so, so if the game relies on the order of
     * libraries by calling dlsym with RTLD_NEXT, it may return the wrong
     * function and an infinite call.

     * We are handling each case here. */
    if (handle == RTLD_NEXT) {
        GlobalNative gn;

        /* Chrome */
        if ((strcmp(name, "localtime") == 0) ||
            (strcmp(name, "localtime64") == 0) ||
            (strcmp(name, "localtime_r") == 0) ||
            (strcmp(name, "localtime64_r") == 0)) {
            void* libc_handle = dlopen("libc.so.6", RTLD_LAZY);
            void* ret = orig::dlsym(libc_handle, name);
            recurs_count--;
            return ret;
        }
        else {
            debuglogstdio(LCF_HOOK | LCF_WARNING, "   dlsym called with RTLD_NEXT for symbol %s!", name);
        }
    }

    /* Detect a Unity game when it loads specific Unity functions */
    if (std::strstr(name, "mono_unity_") != nullptr)
        GameHacks::setUnity();

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

#ifdef __linux__
    if (0 == strcmp(name, "__wine_process_init")) {
        /* Hook wine LdrGetProcedureAddress function */
        hook_ntdll();
    }
#endif

    recurs_count--;
    return addr;
}

}
