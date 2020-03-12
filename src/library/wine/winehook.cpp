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

#include "winehook.h"
#include "../hookpatch.h"
#include "../logging.h"
// #include <sys/mman.h>

namespace libtas {

struct winstring {
    unsigned short Length;
    unsigned short MaximumLength;
    char *Buffer;
};

namespace orig {

static long __stdcall __attribute__((noinline)) LdrGetProcedureAddress(void *module, const winstring *name, unsigned long ord, void **address)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

}

long __stdcall LdrGetProcedureAddress(void *module, const winstring *name,
                                            unsigned long ord, void **address)
{
    if (name) debuglog(LCF_HOOK | LCF_WINE, __func__, " called with function ", name->Buffer);
    else debuglog(LCF_HOOK | LCF_WINE, __func__, " called with ordinal ", ord);
    return orig::LdrGetProcedureAddress(module, name, ord, address);
}

void hook_ntdll()
{
    HOOK_PATCH_ORIG(LdrGetProcedureAddress, "ntdll.dll.so");
}


}
