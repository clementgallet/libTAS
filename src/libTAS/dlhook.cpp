#include "dlhook.h"
#include "logging.h"
#include <cstring>

static struct dlfcn_hook *old_dlfcn_hook;

static struct dlfcn_hook my_dlfcn_hook = {
    dlopen   : my_dlopen,
    dlclose  : my_dlclose,
    dlsym    : my_dlsym,
    dlvsym   : my_dlvsym,
    dlerror  : my_dlerror,
    dladdr   : my_dladdr,
    dladdr1  : my_dladdr1,
    dlinfo   : my_dlinfo,
    dlmopen  : my_dlmopen,
    pad      : {0, 0, 0, 0}
};

static int depth = 0;
void dlenter(void) { if (!depth++) _dlfcn_hook = old_dlfcn_hook; }
void dlleave(void) { if (!--depth) _dlfcn_hook = &my_dlfcn_hook; }
std::string sdlpath;
std::string openalpath;

std::vector<std::string> libraries;

std::string find_lib(const char* library)
{
    for (std::vector<std::string>::const_iterator itr = libraries.begin(); itr != libraries.end(); ++itr)
        if (itr->find(library) != std::string::npos)
            return (*itr);

    std::string emptystring;
    return emptystring;
}

void *my_dlopen(const char *file, int mode, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with file ", file);
    dlenter();
    result = dlopen(file, mode);
    dlleave();
    if (result != NULL) {
        /* Store the successfully opened library */
        std::string filestr(file);
        libraries.push_back(filestr);

        /* Try to identify some libraries we will be using later */
        if (strstr(file, "libSDL2-2") != NULL)
            sdlpath = std::string(file);
        if (strstr(file, "libSDL-1") != NULL)
            sdlpath = std::string(file);
        if (strstr(file, "libopenal") != NULL)
            openalpath = std::string(file);
    }
    return result;
}

int my_dlclose(void *handle) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlclose(handle);
    dlleave();
    return result;
}

void *my_dlsym(void *handle, const char *name, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with function ", name);
    dlenter();
    /* Try to link to an already defined function first */
    result = dlsym(RTLD_DEFAULT, name);
    if (result == NULL)
        result = dlsym(handle, name);
    dlleave();
    return result;
}

void *my_dlvsym(void *handle, const char *name, const char *version, void *dl_caller) {
    void *result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlvsym(handle, name, version);
    dlleave();
    return result;
}

char *my_dlerror(void) {
    char *result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlerror();
    dlleave();
    return result;
}

int my_dladdr(const void *address, Dl_info *info) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dladdr(address, info);
    dlleave();
    return result;
}

int my_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dladdr1(address, info, extra_info, flags);
    dlleave();
    return result;
}

int my_dlinfo(void *handle, int request, void *arg, void *dl_caller) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlinfo(handle, request, arg);
    dlleave();
    return result;
}

void *my_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller) {
    void *result;
    DEBUGLOGCALL(LCF_HOOK);
    dlenter();
    result = dlmopen(nsid, file, mode);
    dlleave();
    return result;
}

__attribute__((constructor))
static void init(void) {
    old_dlfcn_hook = _dlfcn_hook;
    _dlfcn_hook = &my_dlfcn_hook;
}

__attribute__((destructor))
static void fini(void) {
    _dlfcn_hook = old_dlfcn_hook;
}

