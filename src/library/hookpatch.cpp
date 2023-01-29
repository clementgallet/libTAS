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

#include "hookpatch.h"
#include "dlhook.h"
#include "logging.h"
#include <sys/mman.h>
#include <string.h>
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "GlobalState.h"

namespace libtas {

# ifdef __i386__
static const unsigned char jmp_instr[] = {0xff, 0x25};
# elif defined(__x86_64__)
static const unsigned char jmp_instr[] = {0xff, 0x25, 0x00, 0x00, 0x00, 0x00};
# endif

/* Computes the length of a single instruction.
 * Based on a snippet by Nicolas Capens (which he released to the public domain)
 * Taken from Hourglass <https://github.com/Hourglass-Resurrection/Hourglass-Resurrection>
 */
//_declspec(noinline) inline
static int instruction_length(const unsigned char *func)
{
	const unsigned char *funcstart = func;

    //if(*func != 0xCC)
    {
        // Skip prefixes F0h, F2h, F3h, 66h, 67h, D8h-DFh, 2Eh, 36h, 3Eh, 26h, 64h and 65h
        int operandSize = 4;
        int FPU = 0;
        while(*func == 0xF0 ||
              *func == 0xF2 ||
              *func == 0xF3 ||
             (*func & 0xFC) == 0x64 ||
             (*func & 0xF8) == 0xD8 ||
             (*func & 0x7E) == 0x62)
        {
            if(*func == 0x66)
            {
                operandSize = 2;
            }
            else if((*func & 0xF8) == 0xD8)
            {
                FPU = *func++;
                break;
            }

            func++;
        }

        /* Add x86_64 4xh prefixes */
# ifdef __x86_64__
        while((*func & 0xF0) == 0x40)
        {
            func++;
        }
# endif


        // Skip two-byte opcode byte
        bool twoByte = false;
        if(*func == 0x0F)
        {
            twoByte = true;
            func++;
        }

        // Skip opcode byte
        unsigned char opcode = *func++;

        // Skip mod R/M byte
        unsigned char modRM = 0xFF;
        if(FPU)
        {
            if((opcode & 0xC0) != 0xC0)
            {
                modRM = opcode;
            }
        }
        else if(!twoByte)
        {
            if((opcode & 0xC4) == 0x00 ||
               ((opcode & 0xF4) == 0x60 && ((opcode & 0x0A) == 0x02)) || (opcode & 0x09) == 0x9 ||
               (opcode & 0xF0) == 0x80 ||
               ((opcode & 0xF8) == 0xC0 && (opcode & 0x0E) != 0x02) ||
               (opcode & 0xFC) == 0xD0 ||
               (opcode & 0xF6) == 0xF6)
            {
                modRM = *func++;
            }
        }
        else
        {
            if(((opcode & 0xF0) == 0x00 && (opcode & 0x0F) >= 0x04 && (opcode & 0x0D) != 0x0D) ||
               (opcode & 0xF0) == 0x30 ||
               opcode == 0x77 ||
               (opcode & 0xF0) == 0x80 ||
               ((opcode & 0xF0) == 0xA0 && (opcode & 0x07) <= 0x02) ||
               (opcode & 0xF8) == 0xC8)
            {
                // No mod R/M byte
            }
            else
            {
                modRM = *func++;
            }
        }

        // Skip SIB
        if((modRM & 0x07) == 0x04 &&
           (modRM & 0xC0) != 0xC0)
        {
            func += 1;   // SIB
        }

        // Skip displacement
        if((modRM & 0xC5) == 0x05) func += 4;   // Dword displacement, no base
        if((modRM & 0xC0) == 0x40) func += 1;   // Byte displacement
        if((modRM & 0xC0) == 0x80) func += 4;   // Dword displacement

        // Skip immediate
        if(FPU)
        {
            // Can't have immediate operand
        }
        else if(!twoByte)
        {
            if((opcode & 0xC7) == 0x04 ||
               (opcode & 0xFE) == 0x6A ||   // PUSH/POP/IMUL
               (opcode & 0xF0) == 0x70 ||   // Jcc
               opcode == 0x80 ||
               opcode == 0x83 ||
               (opcode & 0xFD) == 0xA0 ||   // MOV
               opcode == 0xA8 ||            // TEST
               (opcode & 0xF8) == 0xB0 ||   // MOV
               (opcode & 0xFE) == 0xC0 ||   // RCL
               opcode == 0xC6 ||            // MOV
               opcode == 0xCD ||            // INT
               (opcode & 0xFE) == 0xD4 ||   // AAD/AAM
               (opcode & 0xF8) == 0xE0 ||   // LOOP/JCXZ
               opcode == 0xEB ||
               (opcode == 0xF6 && (modRM & 0x30) == 0x00))   // TEST
            {
                func += 1;
            }
            else if((opcode & 0xF7) == 0xC2)
            {
                func += 2;   // RET
            }
            else if((opcode & 0xFC) == 0x80 ||
                    (opcode & 0xC7) == 0x05 ||
                    (opcode & 0xF8) == 0xB8 ||
                    (opcode & 0xFE) == 0xE8 ||      // CALL/Jcc
                    (opcode & 0xFE) == 0x68 ||
                    (opcode & 0xFC) == 0xA0 ||
                    (opcode & 0xEE) == 0xA8 ||
                    opcode == 0xC7 ||
                    (opcode == 0xF7 && (modRM & 0x30) == 0x00))
            {
                func += operandSize;
            }
        }
        else
        {
            if(opcode == 0xBA ||            // BT
               opcode == 0x0F ||            // 3DNow!
               (opcode & 0xFC) == 0x70 ||   // PSLLW
               (opcode & 0xF7) == 0xA4 ||   // SHLD
               opcode == 0xC2 ||
               opcode == 0xC4 ||
               opcode == 0xC5 ||
               opcode == 0xC6)
            {
                func += 1;
            }
            else if((opcode & 0xF0) == 0x80)
            {
                func += operandSize;   // Jcc -i
            }
        }
	}

	return (int)(func - funcstart);
}

void hook_patch(const char* name, const char* library, void* tramp_function, void* my_function)
{
    const char* libpathstr = NULL;
    void* handle;
    
    if (library) {
        /* Find the path to the library */
        std::string libpath = find_lib(library);
        
        if (libpath.empty()) {
            debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not find %s path", library);
            return;
        }

        libpathstr = libpath.c_str();
    }

    /* Open library */
    NATIVECALL(handle = dlopen(libpathstr, RTLD_LAZY));

    if (!handle) {
        debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not load %s", library);
        return;
    }

    /* Load function */
    void *orig_fun;
    NATIVECALL(orig_fun = dlsym(handle, name));

    if (!orig_fun && !library) {
        /* If the symbol was not found, and if looking into the executable,
         * ask libtas program to get the symbol address from the program elf
         * header (much slower). We can't look into our own memory, because
         * the .symtab and .strtab sections of the executable are not loaded
         * in memory by the loader.
         * I don't really want to do it here, because it relies on spawning
         * another process to run `readelf`.
         */
        sendMessage(MSGB_SYMBOL_ADDRESS);
        std::string symstr(name);
        sendString(symstr);

        uint64_t addr;
        receiveData(&addr, sizeof(uint64_t));
        if (addr)
            memcpy(&orig_fun, &addr, sizeof(void*));
        else
            return;
    }

    if (!orig_fun) {
        debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not load %s", name);
        return;
    }

    /* Compute the length of the first instructions to overwrite */
    unsigned char* pOrig = static_cast<unsigned char*>(orig_fun);
    int offset = 0;
# ifdef __i386__
    int min_offset = 10;
# elif defined(__x86_64__)
    int min_offset = 14;
# endif

	while(offset < min_offset) {
        offset += instruction_length(pOrig + offset);
    }

    debuglogstdio(LCF_HOOK, "Saving instructions of length %d", offset);

    /* Overwrite the trampoline function */
    debuglogstdio(LCF_HOOK, "Building our trampoline function in %p", tramp_function);

    char *pTramp = reinterpret_cast<char *>(tramp_function);
    uintptr_t addrTramp = reinterpret_cast<uintptr_t>(pTramp);
    uintptr_t alignedBeg = (addrTramp / 4096) * 4096;
    uintptr_t alignedEnd = ((addrTramp+offset+sizeof(jmp_instr)+sizeof(uintptr_t)) / 4096) * 4096;
    size_t alignedSize = alignedEnd - alignedBeg + 4096;

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ | PROT_WRITE) == 0)

    memcpy(pTramp, orig_fun, offset);
    pTramp += offset;
    memcpy(pTramp, jmp_instr, sizeof(jmp_instr));
    pTramp += sizeof(jmp_instr);
#ifdef __i386__
    uintptr_t indirAddr = reinterpret_cast<uintptr_t>(pTramp+4);
    memcpy(pTramp, &indirAddr, sizeof(uintptr_t));
    pTramp += sizeof(uintptr_t);
#endif
    uintptr_t targetAddr = reinterpret_cast<uintptr_t>(orig_fun)+offset;
    memcpy(pTramp, &targetAddr, sizeof(uintptr_t));

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ) == 0)

    /* Overwrite the original function */
    debuglogstdio(LCF_HOOK, "Overwriting the native function in %p", orig_fun);

    char *pTarget = reinterpret_cast<char *>(orig_fun);
    uintptr_t addrTarget = reinterpret_cast<uintptr_t>(pTarget);
    alignedBeg = (addrTarget / 4096) * 4096;
    alignedEnd = ((addrTarget+sizeof(jmp_instr)+sizeof(uintptr_t)) / 4096) * 4096;
    alignedSize = alignedEnd - alignedBeg + 4096;

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ | PROT_WRITE) == 0)

    memcpy(pTarget, jmp_instr, sizeof(jmp_instr));
    pTarget += sizeof(jmp_instr);
#ifdef __i386__
    indirAddr = reinterpret_cast<uintptr_t>(pTarget+4);
    memcpy(pTarget, &indirAddr, sizeof(uintptr_t));
    pTarget += sizeof(uintptr_t);
#endif
    targetAddr = reinterpret_cast<uintptr_t>(my_function);
    memcpy(pTarget, &targetAddr, sizeof(uintptr_t));

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ) == 0)
}

}
