#include "dlhook.h"
#include "logging.h"

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
static void enter(void) { if (!depth++) _dlfcn_hook = old_dlfcn_hook; }
static void leave(void) { if (!--depth) _dlfcn_hook = &my_dlfcn_hook; }

void *my_dlopen(const char *file, int mode, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with file ", file);
    enter();
    result = dlopen(file, mode);
    leave();
    return result;
}

int my_dlclose(void *handle) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dlclose(handle);
    leave();
    return result;
}

void *my_dlsym(void *handle, const char *name, void *dl_caller) {
    void *result;
    debuglog(LCF_HOOK, __func__, " call with function ", name);
    enter();
    result = dlsym(handle, name);
    leave();
    return result;
}

void *my_dlvsym(void *handle, const char *name, const char *version, void *dl_caller) {
    void *result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dlvsym(handle, name, version);
    leave();
    return result;
}

char *my_dlerror(void) {
    char *result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dlerror();
    leave();
    return result;
}

int my_dladdr(const void *address, Dl_info *info) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dladdr(address, info);
    leave();
    return result;
}

int my_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dladdr1(address, info, extra_info, flags);
    leave();
    return result;
}

int my_dlinfo(void *handle, int request, void *arg, void *dl_caller) {
    int result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dlinfo(handle, request, arg);
    leave();
    return result;
}

void *my_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller) {
    void *result;
    DEBUGLOGCALL(LCF_HOOK);
    enter();
    result = dlmopen(nsid, file, mode);
    leave();
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

