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
#include "dlhook.h"
#include "logging.h"
#include <sys/mman.h>

namespace libtas {

# ifdef __i386__
static unsigned char lgpa_beg[] = {0x8d, 0x4c, 0x24, 0x04, 0x83, 0xe4, 0xf0, 0xff, 0x71, 0xfc};
# elif defined(__x86_64__)
static unsigned char lgpa_beg[] = {0x55, 0x48, 0x89, 0xe5, 0x41, 0x56, 0x45, 0x89, 0xc6, 0x41, 0x55, 0x4d, 0x89, 0xcd};
# endif

static const unsigned char jmp_instr[] = {0xff, 0x25, 0x00, 0x00, 0x00, 0x00};

namespace orig {

static long __stdcall __attribute__((noinline)) LdrGetProcedureAddress(void *module, const winstring *name, unsigned long ord, void **address)
{
    static long x__ = 0;
    x__++;
    x__++;
    if (x__==2) {
        debuglog(LCF_HOOK | LCF_ERROR, "Function got called before it was set up!");
    }
    x__++;
    x__++;
    return 0;
}

}

void hook_ntdll()
{
    /* Find the path to ntdll.dll.so */
    std::string ntdllpath = find_lib("ntdll.dll.so");

    void* handle;
    if (ntdllpath.empty()) {
        debuglog(LCF_HOOK | LCF_ERROR, "Could not find ntdll.dll.so path");
        return;
    }

    /* Open ntdll.dll.so */
    NATIVECALL(handle = dlopen(ntdllpath.c_str(), RTLD_LAZY));

    if (!handle) {
        debuglog(LCF_HOOK | LCF_ERROR, "Could not load ntdll.dll.so");
        return;
    }

    /* Load LdrGetProcedureAddress() */
    void *orig_fun;
    NATIVECALL(orig_fun = dlsym(handle, "LdrGetProcedureAddress"));

    if (!orig_fun) {
        debuglog(LCF_HOOK | LCF_ERROR, "Could not load LdrGetProcedureAddress");
        return;
    }

    if (0 != memcmp(orig_fun, lgpa_beg, sizeof(lgpa_beg))) {
        debuglog(LCF_HOOK | LCF_ERROR, "Beginning of LdrGetProcedureAddress is different from expected");
        return;
    }

    /* Overwrite the trampoline function */
    debuglog(LCF_HOOK, "Building our trampoline function");

    char *pTramp = reinterpret_cast<char *>(orig::LdrGetProcedureAddress);
    intptr_t addrTramp = reinterpret_cast<intptr_t>(pTramp);
    intptr_t alignedBeg = (addrTramp / 4096) * 4096;
    intptr_t alignedEnd = ((addrTramp+sizeof(lgpa_beg)+sizeof(jmp_instr)+sizeof(intptr_t)) / 4096) * 4096;
    size_t alignedSize = alignedEnd - alignedBeg + 4096;

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ | PROT_WRITE) == 0)

    memcpy(pTramp, lgpa_beg, sizeof(lgpa_beg));
    pTramp += sizeof(lgpa_beg);
    memcpy(pTramp, jmp_instr, sizeof(jmp_instr));
    pTramp += sizeof(jmp_instr);
    *(reinterpret_cast<intptr_t*>(pTramp)) = reinterpret_cast<intptr_t>(orig_fun)+sizeof(lgpa_beg);

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ) == 0)

    /* Overwrite the original function */
    debuglog(LCF_HOOK, "Overwriting the native function");

    char *pTarget = reinterpret_cast<char *>(orig_fun);
    intptr_t addrTarget = reinterpret_cast<intptr_t>(pTarget);
    alignedBeg = (addrTarget / 4096) * 4096;
    alignedEnd = ((addrTarget+sizeof(jmp_instr)+sizeof(intptr_t)) / 4096) * 4096;
    alignedSize = alignedEnd - alignedBeg + 4096;

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ | PROT_WRITE) == 0)

    memcpy(pTarget, jmp_instr, sizeof(jmp_instr));
    pTarget += sizeof(jmp_instr);
    *(reinterpret_cast<intptr_t*>(pTarget)) = reinterpret_cast<intptr_t>(LdrGetProcedureAddress);

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ) == 0)
}

long __stdcall LdrGetProcedureAddress(void *module, const winstring *name,
                                            unsigned long ord, void **address)
{
    if (name) debuglog(LCF_HOOK, __func__, " called with function ", name->Buffer);
    else debuglog(LCF_HOOK, __func__, " called with ordinal ", ord);
    return orig::LdrGetProcedureAddress(module, name, ord, address);
}

}
