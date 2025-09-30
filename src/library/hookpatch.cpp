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

struct instr_info {
    bool is_fpu;
    bool operand_size_prefix; // 0x66 prefix
    bool quad_prefix; // REX.W 0x48 prefix for 64-bit operand
    bool multibyte_opcode; // either one-byte, two-byte (0F op) or three bytes (0F 38 op or 0F 3A op)
    unsigned char multibyte_opcode_prefix; // either 0x0F for two-byte, 0x38 or 0x3A for three-byte opcodes
    bool has_modRM;
    unsigned char modRM;
    bool has_sib;
    unsigned char sib;
    unsigned char opcode;
};

/* Computes the length of a single instruction.
 * Based on a snippet by Nicolas Capens (which he released to the public domain)
 * Taken from Hourglass <https://github.com/Hourglass-Resurrection/Hourglass-Resurrection>
 */
//_declspec(noinline) inline
static int instruction_length(const unsigned char *func, instr_info *instr)
{
	const unsigned char *funcstart = func;

    // Create a struct if non passed
    instr_info local_instr;
    if (!instr) {
        instr = &local_instr;
    }

    // Default values
    instr->is_fpu = false;
    instr->operand_size_prefix = false;
    instr->quad_prefix = false;
    instr->multibyte_opcode = false;

    int operandSizeDouble = 4; // operand size for instructions that depend only on 16-bit prefix, and cannot be promoted by REX.W 64-bit
    int operandSize = 4; // operand size for instructions that depend on both operand-size prefixes
    
    // Skip prefixes F0h, F2h, F3h, 66h, 67h, D8h-DFh, 2Eh, 36h, 3Eh, 26h, 64h and 65h
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
            instr->operand_size_prefix = true;
            operandSize = 2;
            operandSizeDouble = 2;
        }
        else if((*func & 0xF8) == 0xD8)
        {
            instr->is_fpu = true;
            FPU = *func++;
            break;
        }

        func++;
    }

    /* Add x86_64 4xh prefixes */
# ifdef __x86_64__
    while((*func & 0xF0) == 0x40)
    {
        if (*func & 0b00001000) {
            instr->quad_prefix = true;
            operandSize = 8;
        }
        func++;
    }
# endif

    // Skip two-byte opcode byte
    if(*func == 0x0F)
    {
        instr->multibyte_opcode = true;
        instr->multibyte_opcode_prefix = 0x0F;
        func++;
    }

    // Skip three-byte opcode byte
    if (instr->multibyte_opcode && ((*func == 0x38) || (*func == 0x3A)))
    {
        instr->multibyte_opcode_prefix = *func;
        func++;
    }

    // Skip opcode byte
    instr->opcode = *func++;

    // Skip mod R/M byte
    instr->has_modRM = false;
    if (instr->is_fpu)
    {
        if((instr->opcode & 0xC0) != 0xC0)
        {
            instr->has_modRM = true;
            instr->modRM = instr->opcode;
        }
    }
    else if(!instr->multibyte_opcode)
    {
        switch (instr->opcode) {
            case 0x62:
            case 0x63:
            case 0x69:
            case 0x6B:
            case 0xC0:
            case 0xC1:
            case 0xC4:
            case 0xC5:
            case 0xC6:
            case 0xC7:
            case 0xF6:
            case 0xF7:
            case 0xFE:
            case 0xFF:
                instr->has_modRM = true;
                instr->modRM = *func++;
                break;
            default:
                // groups of opcodes for smaller code
                if ((instr->opcode & 0xC4) == 0x00 ||
                    (instr->opcode & 0xF0) == 0x80 ||
                    (instr->opcode & 0xFC) == 0xD0 ||
                    (instr->opcode & 0xF6) == 0xF6)
                {
                    instr->has_modRM = true;
                    instr->modRM = *func++;
                }
        }
    }
    else if (instr->multibyte_opcode_prefix == 0x0F) // two-byte opcodes
    {
        switch (instr->opcode) {
            case 0x06:
            case 0x08:
            case 0x09:
            case 0x0B:
            case 0x0D:
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            case 0x35:
            case 0x37:
            case 0xA0:
            case 0xA1:
            case 0xA2:
            case 0xA8:
            case 0xA9:
            case 0xAA:
                // No mod R/M byte
                break;
            default:
                if ((instr->opcode & 0xF8) == 0xC8)
                    break; // No mod R/M byte
                if ((instr->opcode & 0xF0) == 0x80)
                    break; // No mod R/M byte

                instr->has_modRM = true;
                instr->modRM = *func++;
                break;
        }
    }
    else { // three-byte opcodes
        instr->has_modRM = true;
        instr->modRM = *func++;
    }

    // Process modRM
    if (instr->has_modRM) {
        int modRM_mod = (instr->modRM & 0b11000000) >> 6;
        int modRM_rm = instr->modRM & 0b00000111;
        
        // Process SIB
        instr->has_sib = false;
        if ((modRM_mod != 0b11) && (modRM_rm == 0b100))
        {
            instr->has_sib = true;
            instr->sib = *func++;
            int sib_base = instr->sib & 0b00000111;

            // Skip displacement
            if ((modRM_mod == 0b00) && (sib_base == 0b101)) func += 4;   // Dword displacement with SIB
        }
        
        if ((modRM_mod == 0b00) && (modRM_rm == 0b101)) func += 4;   // Dword displacement with base
        if  (modRM_mod == 0b01) func += 1;   // Byte displacement
        if  (modRM_mod == 0b10) func += 4;   // Dword displacement
    }

    // Skip immediate
    if (instr->is_fpu)
    {
        // Can't have immediate operand
    }
    else if(!instr->multibyte_opcode)
    {
        switch (instr->opcode) {
            // imm8
            case 0x04: // ADD
            case 0x0C: // OR
            case 0x14: // ADC
            case 0x1C: // SBB
            case 0x24: // AND
            case 0x2C: // SUB
            case 0x34: // XOR
            case 0x3C: // CMP
            case 0x6A: // PUSH
            case 0x6B: // IMUL
            case 0x80:
            case 0x82:
            case 0x83:
            case 0xA8: // TEST
            case 0xB0: // MOV
            case 0xB1: // MOV
            case 0xB2: // MOV
            case 0xB3: // MOV
            case 0xB4: // MOV
            case 0xB5: // MOV
            case 0xB6: // MOV
            case 0xB7: // MOV
            case 0xC0: // 
            case 0xC1: // 
            case 0xC6: // MOV
            case 0xCD: // INT
            case 0xD4: // AMX
            case 0xD5: // ADX
            case 0xE4: // IN
            case 0xE5: // IN
            case 0xE6: // OUT
            case 0xE7: // OUT
            // rel8
            case 0x70: // JO
            case 0x71: // JNO
            case 0x72: // JB
            case 0x73: // JNB
            case 0x74: // JZ
            case 0x75: // JNZ
            case 0x76: // JBE
            case 0x77: // JNBE
            case 0x78: // JS
            case 0x79: // JNS
            case 0x7A: // JP
            case 0x7B: // JNP
            case 0x7C: // JL
            case 0x7D: // JNL
            case 0x7E: // JLE
            case 0x7F: // JNLE
            case 0xE0: // LOOPNZ
            case 0xE1: // LOOPZ
            case 0xE2: // LOOP
            case 0xE3: // JCXZ
            case 0xEB: // JMP
            // moffs8
            case 0xA0: // MOV
            case 0xA2: // MOV
                func += 1;
                break;

            case 0xF6:
                if ((instr->modRM & 0x30) == 0x00) // TEST
                    func += 1;
                break;
        
            // imm16
            case 0xC2: // RETN
            case 0xCA: // RETF
                func += 2;   // RET
                break;
                
            // imm16/32
            case 0x05: // ADD
            case 0x0D: // OR
            case 0x15: // ADC
            case 0x1D: // SBB
            case 0x25: // AND
            case 0x2D: // SUB
            case 0x35: // XOR
            case 0x3D: // CMP
            case 0x68: // PUSH
            case 0x69: // IMUL
            case 0x81:
            case 0xA9: // TEST
            case 0xC7: // MOV
            // rel16/32
            case 0xE8: // CALL
            case 0xE9: // JMP
                func += operandSizeDouble;
                break;
            
            // imm16/32/64
            case 0xB8: // MOV
            case 0xB9: // MOV
            case 0xBA: // MOV
            case 0xBB: // MOV
            case 0xBC: // MOV
            case 0xBD: // MOV
            case 0xBE: // MOV
            case 0xBF: // MOV
            // moffs16/32/64
            case 0xA1: // MOV
            case 0xA3: // MOV
                func += operandSize;
                break;
            
            case 0xF7:
                if ((instr->modRM & 0x30) == 0x00) // TEST imm16/32
                    func += operandSizeDouble;
                break;
        }
    }
    else if (instr->multibyte_opcode_prefix == 0x0F) // two-byte opcodes
    {
        switch (instr->opcode) {
            case 0x0F: // 3DNow!
            case 0x70: // PS
            case 0x71: // PS
            case 0x72: // PS
            case 0x73: // PS
            case 0xA4: // SHLD
            case 0xAC: // SHRD
            case 0xBA: // BT
            case 0xC2: // CMP
            case 0xC4: // PINSRW
            case 0xC5: // PEXTRW
            case 0xC6: // SHUFP
                func += 1;
                break;
            case 0x80: // JO
            case 0x81: // JNO
            case 0x82: // JB
            case 0x83: // JNB
            case 0x84: // JZ
            case 0x85: // JNZ
            case 0x86: // JBE
            case 0x87: // JNBE
            case 0x88: // JS
            case 0x89: // JNS
            case 0x8A: // JP
            case 0x8B: // JNP
            case 0x8C: // JL
            case 0x8D: // JNL
            case 0x8E: // JLE
            case 0x8F: // JNLE
                func += operandSizeDouble;
                break;
        }
    }
    else if (instr->multibyte_opcode_prefix == 0x3A) {
        switch (instr->opcode) {
            case 0x08: // ROUNDPS
            case 0x09: // ROUNDPD
            case 0x0A: // ROUNDSS
            case 0x0B: // ROUNDSD
            case 0x0C: // BLENDPS
            case 0x0D: // BLENDPD
            case 0x0E: // PBLENDW
            case 0x14: // PEXTRB
            case 0x15: // PEXTRW
            case 0x16: // PEXTRD
            case 0x17: // EXTRACTPS
            case 0x20: // PINSRB
            case 0x21: // INSERTPS
            case 0x22: // PINSRD
            case 0x42: // MPSADBW
            case 0x62: // PCMPISTRM
            case 0x63: // PCMPISTRI
                func += 1;
                break;
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
        offset += instruction_length(pOrig + offset, nullptr);
        
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
    instr_info instr;
    
    while (cur_offset < offset) {
        int instr_len = instruction_length(pOrig + cur_offset, &instr);
        
        std::ostringstream oss;
        
        /* Transcribe each instruction, and update the occasionnal relative address */
        unsigned char opcode = *(pOrig+cur_offset);
        
        if (!instr.multibyte_opcode) {
            
            // Case where modRM with offset relative to the current instruction
            // pointer value. We need to change the offset in place
            bool relative_rip = instr.has_modRM &&
                (instr.modRM & 0b11000000) == 0 &&
                (instr.modRM & 0b00000111) == 0xb00000101;
                
            // Case where instruction has a relative 32-bit operand
            bool relative_op = (instr.opcode == 0xe8) || // CALL
                (instr.opcode == 0xe9); // JMP

            // Case where instruction has a relative 8-bit operand
            bool relative_short = false;
            if ((instr.opcode >= 0x70) && (instr.opcode <= 0x7f))
                relative_short = true;
            if ((instr.opcode >= 0xe0) && (instr.opcode <= 0xe3))
                relative_short = true;
            if (instr.opcode == 0xeb)
                relative_short = true;

            if (relative_rip || relative_op) {
                // TODO: we do not support 16-bit operand!
                if (instr.operand_size_prefix)
                    LOG(LL_WARN, LCF_HOOK, "  Relative address is 16-bit, we do not support that!");

                /* Compute where the relative adress is in the instruction. 
                 * There is no instruction that has a relative 64-bit address,
                 * whatever prefix is present, so we don't need to check for that. */
                int off_to_rel = instr_len - 4;
                
                LOG(LL_DEBUG, LCF_HOOK, "  Found instruction with rel addr %#x", (*reinterpret_cast<const int*>(pOrig+cur_offset+off_to_rel)));
                
                /* Write the instruction and just change the offset to
                * the new offset between our function and the call target. */
                memcpy(currentTrampAddr, pOrig+cur_offset, off_to_rel);
                currentTrampAddr += off_to_rel;
                
                const unsigned char* target_addr = pOrig + cur_offset + instr_len + (*reinterpret_cast<const int*>(pOrig+cur_offset+off_to_rel));
                LOG(LL_DEBUG, LCF_HOOK, "  Absolute addr becomes %p", target_addr);
                
                ptrdiff_t new_offset = reinterpret_cast<ptrdiff_t>(target_addr) - reinterpret_cast<ptrdiff_t>(currentTrampAddr) - 4;
                /* Check if it fits into signed 32-bit */
                if (new_offset < INT32_MIN || new_offset > INT32_MAX) {
                    LOG(LL_ERROR, LCF_HOOK, "  Could not modify instruction, offset too large: %lld", new_offset);
                }
                int32_t new_offset_32 = static_cast<int32_t>(new_offset);
                
                LOG(LL_DEBUG, LCF_HOOK, "  New relative addr becomes %#x", new_offset_32);
                
                memcpy(currentTrampAddr, &new_offset_32, 4);
                currentTrampAddr += 4;
                
                for (int i = 0; i < off_to_rel; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(pOrig+cur_offset+i)) << " ";
                }                
                for (int i = 0; i < 4; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(currentTrampAddr-4+i)) << " ";
                }
                LOG(LL_DEBUG, LCF_HOOK, "  Write modified instruction %s", oss.str().c_str());
            }
            else if (relative_short) {
                LOG(LL_DEBUG, LCF_HOOK, "  Found conditional jump instruction %#hhx to rel addr %#hhx", *(pOrig+cur_offset), (*(pOrig+cur_offset+1)));
                
                /* Compute where the relative adress is in the instruction. */
                int off_to_rel = instr_len - 1;

                jmp_info info;
                info.target_addr = pOrig + cur_offset + instr_len + (*reinterpret_cast<const int8_t*>(pOrig+cur_offset+off_to_rel));
                info.offset_addr = currentTrampAddr + off_to_rel;
                jmp_list.push_back(info);
                
                LOG(LL_DEBUG, LCF_HOOK, "  Absolute addr becomes %p", info.target_addr);
                
                /* Write the same conditional jump, but later change the offset
                * so that it jumps to another jump
                * instruction where we can write a 64-bit address. */
                memcpy(currentTrampAddr, pOrig+cur_offset, instr_len);
                currentTrampAddr += instr_len;
            }
            else {
                memcpy(currentTrampAddr, pOrig + cur_offset, instr_len);
                currentTrampAddr += instr_len;
                
                for (int i = 0; i < instr_len; i++) {
                    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*(pOrig+cur_offset+i)) << " ";
                }
                LOG(LL_DEBUG, LCF_HOOK, "  Write unmodified instruction %s", oss.str().c_str());
            }
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
