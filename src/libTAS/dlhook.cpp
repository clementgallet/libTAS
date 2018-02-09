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

#include "dlhook.h"
#include "logging.h"
#include <cstring>
#include <set>
#include "backtrace.h"

namespace libtas {

/* Set of libraries that are loaded by the game,
 * either at startup (link time) or using the dl functions.
 */
static std::set<std::string> libraries;

// void add_lib(std::string library)
// {
//     if (! libraries)
//         libraries = new std::set<std::string>();
//     libraries->insert(library);
// }

std::string find_lib(const char* library)
{
    for (auto const& itr : libraries)
        if (itr.find(library) != std::string::npos)
            return itr;

    std::string emptystring;
    return emptystring;
}

static void *my_dlopen(const char *file, int mode, void *dl_caller);
static int my_dlclose(void *handle);
static void *my_dlsym(void *handle, const char *name, void *dl_caller);
static void *my_dlvsym(void *handle, const char *name, const char *version, void *dl_caller);
static char *my_dlerror(void);
static int my_dladdr(const void *address, Dl_info *info);
static int my_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags);
static int my_dlinfo(void *handle, int request, void *arg, void *dl_caller);
static void *my_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller);


static void *my_dlopen(const char *file, int mode, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with file ", (file!=nullptr)?file:"<NULL>");
    dlenter();
    result = dlopen(file, mode);
    dlleave();

    if (result && (file != nullptr))
        libraries.insert(std::string(file));
    return result;
}

static int my_dlclose(void *handle) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlclose(handle);
    dlleave();
    return result;
}

static void *my_dlsym(void *handle, const char *name, void *dl_caller) {
    void *addr;
    debuglog(LCF_HOOK, __func__, " call with function ", name);

    /* Special cases when dlsym is called with dl* functions (yes, it happens...).
     * Because our dl* functions have another name (my_dl*), the standard code
     * won't work. So we manually return the correct function pointers.
     */
    if (strcmp(name, "dlopen") == 0)
        return reinterpret_cast<void*>(my_dlopen);
    if (strcmp(name, "dlclose") == 0)
        return reinterpret_cast<void*>(my_dlclose);
    if (strcmp(name, "dlsym") == 0)
        return reinterpret_cast<void*>(my_dlsym);
    if (strcmp(name, "dlvsym") == 0)
        return reinterpret_cast<void*>(my_dlvsym);
    if (strcmp(name, "dlerror") == 0)
        return reinterpret_cast<void*>(my_dlerror);
    if (strcmp(name, "dladdr") == 0)
        return reinterpret_cast<void*>(my_dladdr);
    if (strcmp(name, "dladdr1") == 0)
        return reinterpret_cast<void*>(my_dladdr1);
    if (strcmp(name, "dlinfo") == 0)
        return reinterpret_cast<void*>(my_dlinfo);
    if (strcmp(name, "dlmopen") == 0)
        return reinterpret_cast<void*>(my_dlmopen);

    dlenter();
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

    /* Correct way of using dlsym is with dlerror */
    dlerror(); // Reseting the internal buffer

    /* Try to link to an already defined function first */
    addr = dlsym(RTLD_DEFAULT, name);
    char* error = dlerror();
    if (error == nullptr) {
        /* We found a matching symbol. Now checking that this symbol comes
         * from our library and not another linked library.
         */
         Dl_info info;
         int res = dladdr(addr, &info);
         if (res != 0) {
             std::string libpath = info.dli_fname;
             std::string libtasstr = "libTAS.so"; // bad!
             if (libpath.length() >= libtasstr.length() &&
                libpath.compare(libpath.length()-libtasstr.length(), libtasstr.length(), libtasstr) == 0) {
                debuglog(LCF_HOOK, "   function ", name, " is overriden!");
                dlleave();
                return addr;
            }
        }
    }

    addr = dlsym(handle, name);
    dlleave();
    return addr;
}

static void *my_dlvsym(void *handle, const char *name, const char *version, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with function ", name, " and version ", version);
    dlenter();
    result = dlvsym(handle, name, version);
    dlleave();
    return result;
}

static char *my_dlerror(void) {
    char *result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlerror();
    dlleave();
    return result;
}

static int my_dladdr(const void *address, Dl_info *info) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dladdr(address, info);
    dlleave();
    return result;
}

static int my_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dladdr1(address, info, extra_info, flags);
    dlleave();
    return result;
}

static int my_dlinfo(void *handle, int request, void *arg, void *dl_caller) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlinfo(handle, request, arg);
    dlleave();
    return result;
}

static void *my_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with file ", (file!=nullptr)?file:"<NULL>");
    dlenter();
    result = dlmopen(nsid, file, mode);
    dlleave();
    return result;
}

static struct dlfcn_hook *old_dlfcn_hook;

static struct dlfcn_hook my_dlfcn_hook = {
    my_dlopen, // dlopen
    my_dlclose, // dlclose
    my_dlsym, // dlsym
    my_dlvsym, // dlvsym
    my_dlerror, // dlerror
    my_dladdr, // dladdr
    my_dladdr1, // dladdr1
    my_dlinfo, // dlinfo
    my_dlmopen, // dlmopen
    {0, 0, 0, 0} // pad
};

static int depth;
void dlenter(void) { if (!depth++) _dlfcn_hook = old_dlfcn_hook; }
void dlleave(void) { if (!--depth) _dlfcn_hook = &my_dlfcn_hook; }

void dlhook_init(void)
{
    static int inited = 0;
    if (!inited) {
        old_dlfcn_hook = _dlfcn_hook;
        _dlfcn_hook = &my_dlfcn_hook;
        inited = 1;
    }
}

void dlhook_end(void)
{
    _dlfcn_hook = old_dlfcn_hook;
}

}
