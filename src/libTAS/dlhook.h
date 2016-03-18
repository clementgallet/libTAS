#ifndef DLHOOK_H_INCLUDED
#define DLHOOK_H_INCLUDED

#include <dlfcn.h>
#include <string>
#include <vector>

/* For hooking functions that perform dynamic library loading,
 * we have to use a special method.
 * Indeed, we cannot use dlsym to get the original function
 * because we are hooking dlsym itself.
 * Solution comes from this post:
 * http://stackoverflow.com/a/1161195
 * This solution is very specific to glibc, by replacing
 * the internal struct of dl functions
 */

extern struct dlfcn_hook {
    void *(*dlopen)(const char *, int, void *);
    int (*dlclose)(void *);
    void *(*dlsym)(void *, const char *, void *);
    void *(*dlvsym)(void *, const char *, const char *, void *);
    char *(*dlerror)(void);
    int (*dladdr)(const void *, Dl_info *);
    int (*dladdr1)(const void *, Dl_info *, void **, int);
    int (*dlinfo)(void *, int, void *, void *);
    void *(*dlmopen)(Lmid_t, const char *, int, void *);
    void *pad[4];
} *_dlfcn_hook;

std::string find_lib(const char* library);

extern std::vector<std::string> libraries;

/* Functions specifying that we want to call the original functions */
void dlenter(void);
void dlleave(void);

/* Custom version of each dl functions */
void *my_dlopen(const char *file, int mode, void *dl_caller);
int my_dlclose(void *handle);
void *my_dlsym(void *handle, const char *name, void *dl_caller);
void *my_dlvsym(void *handle, const char *name, const char *version, void *dl_caller);
char *my_dlerror(void);
int my_dladdr(const void *address, Dl_info *info);
int my_dladdr1(const void *address, Dl_info *info, void **extra_info, int flags);
int my_dlinfo(void *handle, int request, void *arg, void *dl_caller);
void *my_dlmopen(Lmid_t nsid, const char *file, int mode, void *dl_caller);

/* Path of some libraries we will need */
extern std::string sdlpath;
extern std::string openalpath;

#endif
