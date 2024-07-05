/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "general/dlhook.h"
#include "logging.h"
#include "GlobalState.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <sys/mman.h>
#include <string.h>
#include <cstdint>
#include <sstream>
#include <list>

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

/* To convert some instructions from the original function, we need to be at
 * 32-bit offset from the function location. */
static void* allocate_nearby_segment()
{
    /* I'm lazy and assume the function is inside the first 32-bit memory, which
     * is usually the case for main executable */
    
    void* obtainedAddr = mmap(nullptr, 0x1000, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, 0, 0);
    
    if (obtainedAddr == MAP_FAILED) {
        LOG(LL_DEBUG, LCF_HOOK, "  Could not obtain a memory segment for hookpatch functions, error %d", errno);
        return nullptr;
    }
    
    return obtainedAddr;
}

/* Compute how many instructions we need to overwrite in the original function */
static int compute_overwrite_offset(const unsigned char* pOrig)
{
    int offset = 0;
    # ifdef __i386__
    int min_offset = 10;
    # elif defined(__x86_64__)
    int min_offset = 14;
    # endif
    
    int old_offset = 0;
    while(offset < min_offset) {
        offset += instruction_length(pOrig + offset);
        
        std::ostringstream oss;
        for (int off = old_offset; off < offset; off++) {
            oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(pOrig + off)) << " ";
        }
        LOG(LL_DEBUG, LCF_HOOK, "  Found instruction %s", oss.str().c_str());
        
        old_offset = offset;
    }
    LOG(LL_DEBUG, LCF_HOOK, "  Saving instructions of length %d", offset);
    return offset;
}

struct jmp_info {
    unsigned char* offset_addr; // where do we need to write the offset to the jmp address
    const unsigned char* target_addr; // where the jmp instructions goes to
};

/* Fill into pTramp the function pointer of the built trampoline function.
 * This function contains the first instructions of original function,
 * where some instructions needs a modification, and a jump
 * to the remaining instructions of original function.
 */
static void write_tramp_function(const void *orig_fun, void **pTramp)
{
    static unsigned char* currentTrampAddr = nullptr;
    if (!currentTrampAddr) {
        currentTrampAddr = static_cast<unsigned char*>(allocate_nearby_segment());
        
        if (!currentTrampAddr) {
            return;
        }
    }
    
    *pTramp = currentTrampAddr;
    
    const unsigned char* pOrig = static_cast<const unsigned char*>(orig_fun);
    int offset = compute_overwrite_offset(pOrig);
    
    /* Overwrite the trampoline function */
    LOG(LL_DEBUG, LCF_HOOK, "  Building our trampoline function in %p", currentTrampAddr);
    
    /* Write each instruction of the original function that we will overwrite,
     * and update instructions with relative addresses. Our trampoline function
     * is written in an address that is at most 32-bit away from the original
     * function, so that for instructions with 32-bit relative address can be
     * updated in place.
     * Our trampoline will look like that:
     *     orig_instr1
     *     ...
     *     orig_instrN
     *     ff 25 00 00 00 00         jmp *(%rip)
     *     ad dr re ss to or ig fn   address to orig function offset by N bytes
     *
     * For the short jump instructions, we change the offset
     * to jump to another jump instruction
     *     orig_instr1
     *     7e of                     jne 0xof
     *     orig_instrN
     *     ff 25 00 00 00 00         jmp *(%rip)
     *     ad dr re ss to or ig fn   address to orig function offset by N bytes
     *     ad dr re ss to ju mp 01   address to orig called function 01
     */
    
    int cur_offset = 0;    
    std::list<jmp_info> jmp_list;
    
    while (cur_offset < offset) {
        int instr_len = instruction_length(pOrig + cur_offset);
        
        std::ostringstream oss;
        
        /* Transcribe each instruction, and update the occasionnal relative address */
        unsigned char opcode = *(pOrig+cur_offset);
        
        switch (opcode) {
            case 0xe8: // CALL
            case 0x3b: // CMP
            {
                
                int off_to_rel = 0;
                switch (opcode) {
                    case 0xe8: // CALL
                        off_to_rel = 1;
                        break;
                    case 0x3b: // CMP
                        off_to_rel = 2;
                        break;
                }
                
                LOG(LL_DEBUG, LCF_HOOK, "  Found instruction with rel addr %p", (*reinterpret_cast<const int*>(pOrig+cur_offset+off_to_rel)));

                /* Write the instruction and just change the offset to
                 * the new offset between our function and the call target. */
                memcpy(currentTrampAddr, pOrig+cur_offset, off_to_rel);
                currentTrampAddr += off_to_rel;

                const unsigned char* target_addr = pOrig + cur_offset + instr_len + (*reinterpret_cast<const int*>(pOrig+cur_offset+off_to_rel));
                LOG(LL_DEBUG, LCF_HOOK, "  Absolute addr becomes %p", target_addr);
                
                ptrdiff_t new_offset = reinterpret_cast<ptrdiff_t>(target_addr) - reinterpret_cast<ptrdiff_t>(currentTrampAddr) - 4;
                /* Check if it fits into signed 32-bit */
                if (new_offset < INT32_MIN || new_offset > INT32_MAX) {
                    LOG(LL_ERROR, LCF_HOOK, "  Could not modify CALL instruction, offset too large: %lld", new_offset);
                }
                int32_t new_offset_32 = static_cast<int32_t>(new_offset);

                LOG(LL_DEBUG, LCF_HOOK, "  New relative addr becomes %x", new_offset_32);

                memcpy(currentTrampAddr, &new_offset_32, 4);
                currentTrampAddr += 4;
                
                for (int i = 0; i < off_to_rel; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(pOrig+cur_offset+i)) << " ";
                }                
                for (int i = 0; i < 4; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(currentTrampAddr-4+i)) << " ";
                }
                LOG(LL_DEBUG, LCF_HOOK, "  Write modified instruction %s", oss.str().c_str());

                break;
            }
            
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x75:
            case 0x76:
            case 0x77:
            case 0x78:
            case 0x79:
            case 0x7a:
            case 0x7b:
            case 0x7c:
            case 0x7d:
            case 0x7e:
            case 0x7f:
            case 0xe3:
            {
                LOG(LL_DEBUG, LCF_HOOK, "  Found conditional jump instruction %p to rel addr %p", *(pOrig+cur_offset), (*(pOrig+cur_offset+1)));

                jmp_info info;
                info.target_addr = pOrig + cur_offset + instr_len + (*reinterpret_cast<const int8_t*>(pOrig+cur_offset+1));
                info.offset_addr = currentTrampAddr + 1;
                jmp_list.push_back(info);

                LOG(LL_DEBUG, LCF_HOOK, "  Absolute addr becomes %p", info.target_addr);

                /* Write the same conditional jump, but later change the offset
                 * so that it jumps to another jump
                 * instruction where we can write a 64-bit address. */
                memcpy(currentTrampAddr, pOrig+cur_offset, 2);
                currentTrampAddr += 2;

                break;
            }
            default:
                memcpy(currentTrampAddr, pOrig + cur_offset, instr_len);
                currentTrampAddr += instr_len;
                
                for (int i = 0; i < instr_len; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(pOrig+cur_offset+i)) << " ";
                }
                LOG(LL_DEBUG, LCF_HOOK, "  Write unmodified instruction %s", oss.str().c_str());

                break;
        }
        cur_offset += instr_len;
    }
    
    /* Write the jmp instruction to the orginal function */
    memcpy(currentTrampAddr, jmp_instr, sizeof(jmp_instr));
    currentTrampAddr += sizeof(jmp_instr);
#ifdef __i386__
    uintptr_t indirAddr = reinterpret_cast<uintptr_t>(currentTrampAddr+4);
    memcpy(currentTrampAddr, &indirAddr, sizeof(uintptr_t));
    currentTrampAddr += sizeof(uintptr_t);
#endif
    uintptr_t targetAddr = reinterpret_cast<uintptr_t>(pOrig)+offset;
    memcpy(currentTrampAddr, &targetAddr, sizeof(uintptr_t));
    currentTrampAddr += sizeof(uintptr_t);

    /* Write all the call target functions, and edit the offsets */
    for (auto info = jmp_list.begin(); info != jmp_list.end(); info++) {
        /* Write 8-bit offset */
        int8_t off = reinterpret_cast<ptrdiff_t>(currentTrampAddr) - reinterpret_cast<ptrdiff_t>(info->offset_addr) - 1;
        memcpy(info->offset_addr, &off, sizeof(int8_t));
        
        /* Write JMP instruction */
        memcpy(currentTrampAddr, jmp_instr, sizeof(jmp_instr));
        currentTrampAddr += sizeof(jmp_instr);
#ifdef __i386__
        uintptr_t indirAddr = reinterpret_cast<uintptr_t>(currentTrampAddr+4);
        memcpy(currentTrampAddr, &indirAddr, sizeof(uintptr_t));
        currentTrampAddr += sizeof(uintptr_t);
#endif

        /* Write called function absolute address */
        const unsigned char *addr = info->target_addr;
        memcpy(currentTrampAddr, &addr, sizeof(uintptr_t));
        currentTrampAddr += sizeof(uintptr_t);
    }
}

void overwrite_orig_function(void *orig_fun, void* my_function)
{
    /* Overwrite the original function */
    LOG(LL_DEBUG, LCF_HOOK, "  Overwriting the native function in %p", orig_fun);

    char *pTarget = reinterpret_cast<char *>(orig_fun);
    uintptr_t addrTarget = reinterpret_cast<uintptr_t>(pTarget);
    uintptr_t alignedBeg = (addrTarget / 4096) * 4096;
    uintptr_t alignedEnd = ((addrTarget+sizeof(jmp_instr)+sizeof(uintptr_t)) / 4096) * 4096;
    size_t alignedSize = alignedEnd - alignedBeg + 4096;

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ | PROT_WRITE) == 0)

    memcpy(pTarget, jmp_instr, sizeof(jmp_instr));
    pTarget += sizeof(jmp_instr);
#ifdef __i386__
    uintptr_t indirAddr = reinterpret_cast<uintptr_t>(pTarget+4);
    memcpy(pTarget, &indirAddr, sizeof(uintptr_t));
    pTarget += sizeof(uintptr_t);
#endif
    uintptr_t targetAddr = reinterpret_cast<uintptr_t>(my_function);
    memcpy(pTarget, &targetAddr, sizeof(uintptr_t));

    MYASSERT(mprotect(reinterpret_cast<void*>(alignedBeg), alignedSize, PROT_EXEC | PROT_READ) == 0)
}

void hook_patch(const char* name, const char* library, void** tramp_function, void* my_function)
{
    const char* libpathstr = NULL;
    void* handle;
    
    if (library) {
        /* Find the path to the library */
        std::string libpath = find_lib(library);
        
        if (libpath.empty()) {
            LOG(LL_ERROR, LCF_HOOK, "Could not find %s path", library);
            return;
        }

        libpathstr = libpath.c_str();
    }

    /* Open library */
    NATIVECALL(handle = dlopen(libpathstr, RTLD_LAZY));

    if (!handle) {
        LOG(LL_ERROR, LCF_HOOK, "Could not load %s", library);
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
    }

    if (!orig_fun)
        return;

    LOG(LL_DEBUG, LCF_HOOK, "Patching function %s", name);

    hook_patch_addr(orig_fun, tramp_function, my_function);
}

void hook_patch_addr(void *orig_fun, void** tramp_function, void* my_function)
{
    write_tramp_function(orig_fun, tramp_function);
    overwrite_orig_function(orig_fun, my_function);
}

}
