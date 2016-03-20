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

#ifndef DLHOOK_H_INCLUDED
#define DLHOOK_H_INCLUDED

#include <dlfcn.h>
#include <string>
#include <vector>

/* There are two ways a program can link to a shared library:
 * either it was compiled with an include of the library header foo.h
 * and using the library as an argument (-lfoo)
 * or it can link using the set of dl functions (dlopen, dlsym, etc.)
 * during the program runtime.
 *
 * In the first case, the functions of the shared library that the game
 * is using are included in the list of symbols of the executable,
 * so by writing a function with the same name and using the LD_PRELOAD trick,
 * the game will call our function instead of the original.
 *
 * However, in the second case, the game declare a function pointer,
 * and link the function inside the library to it at runtime.
 * Then the game can call the function using its pointer.
 * In this case, writing a function with the same name is not enough.
 *
 * One solution to this is to hook the functions that perform
 * the dynamic library loading. Then, if the game try to load
 * a function from a shared library, we will first try to load
 * the function from inside the program first. If we found one,
 * it is probably our custom function. By linking to it,
 * the program will call our function instead of the function
 * from the shared library.

 * For hooking functions that perform dynamic library loading,
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

#endif
