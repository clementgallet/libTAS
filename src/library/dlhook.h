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

#ifndef LIBTAS_DLHOOK_H_INCLUDED
#define LIBTAS_DLHOOK_H_INCLUDED

#include <dlfcn.h>
#include <string>
#include "global.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

namespace libtas {

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
 * https://stackoverflow.com/a/18825060
 * which is using the internal _dl_sym function for dynamic library loading
 * However, this internal function is much more unsafe, so we are only using it
 * to access to the real dlsym function, and we use that function everything else.
 */

/* Locate a library path in the above set from a substring,
 * and returns the first match.
 */
std::string find_lib(const char* library);

/* Add a library path to the set */
void add_lib(const char* library);

/* Try to locate a symbol.  If original is true then only return
 * symbols that are not from libtas.so, otherwise only return
 * symbols that are from libtas.so.
 */
void *find_sym(const char* name, bool original = false);

OVERRIDE void *dlopen(const char *file, int mode) __THROW;
OVERRIDE void *dlsym(void *handle, const char *name) __THROW;

#ifdef __unix__
/* Declare internal implementation-dependent dlsym function.
 * It is declared as weak, so we have a fallback if not present */
OVERRIDE void *_dl_sym(void *, const char *, void *) __attribute__((weak));
#elif defined(__APPLE__) && defined(__MACH__)
/* Declare internal function to locate the address of a dyld function */
OVERRIDE int _dyld_func_lookup(const char* name, void** address);
#endif

}

#endif
