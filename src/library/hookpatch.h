/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_HOOKPATCH_H_INCLUDED
#define LIBTAS_HOOKPATCH_H_INCLUDED

#include "logging.h"

namespace libtas {

/* Hook a function by changing the first instructions of the function with
 * a jmp instruction to our function. To call back the original function,
 * we save those instructions into a trampoline function, which then jumps to
 * the original function.
 */
void hook_patch(const char* name, const char* library, void** tramp_function, void* my_function);

/* Hook with the target function address instead of the symbol name */
void hook_patch_addr(void *orig_fun, void** tramp_function, void* my_function);

#define HOOK_PATCH_ORIG(FUNC,LIB) hook_patch(#FUNC, LIB, reinterpret_cast<void**>(&orig::FUNC), reinterpret_cast<void*>(FUNC))

#define HOOK_PLACEHOLDER_RETURN_ZERO \
    static long x__ = 0;\
    x__++;\
    x__++;\
    if (x__==2) {\
        debuglogstdio(LCF_HOOK | LCF_ERROR, "Function got called before it was set up!");\
    }\
    x__++;\
    x__++;\
    return 0;

}

#endif
