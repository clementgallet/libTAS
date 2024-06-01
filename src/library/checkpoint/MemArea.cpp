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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "MemArea.h"
#include "ReservedMemory.h"

#include "logging.h"
#include "Utils.h"

#include <unistd.h>
#include <sys/mman.h> // PROT_READ, PROT_WRITE, etc.
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/vm_prot.h> // VM_PROT_READ, VM_PROT_WRITE, etc.
#endif

namespace libtas {

void Area::print(const char* prefix) const
{
#ifdef __unix__
    LOG(LL_DEBUG, LCF_CHECKPOINT, "%s Region %c%c%c%c %p-%p (%s) with size %zu and flags %x",
    prefix,
    (prot&PROT_READ)?'r':'-', (prot&PROT_WRITE)?'w':'-', (prot&PROT_EXEC)?'x':'-', (flags&AREA_SHARED)?'s':'p',
    addr, endAddr, name, size, flags);
#elif defined(__APPLE__) && defined(__MACH__)
    LOG(LL_DEBUG, LCF_CHECKPOINT, "%s Region %c%c%c%c %p-%p (%s) with size %zu and flags %x",
    prefix,
    (prot&VM_PROT_READ)?'r':'-', (prot&VM_PROT_WRITE)?'w':'-', (prot&VM_PROT_EXECUTE)?'x':'-', (flags&AREA_SHARED)?'s':'p',
    addr, endAddr, name, size, flags);
#endif
}

int Area::toMmapFlag() const
{
    int mf = MAP_FIXED;
    if (flags & AREA_ANON)
        mf |= MAP_ANON;
    if (flags & AREA_PRIV)
        mf |= MAP_PRIVATE;
    if (flags & AREA_SHARED)
        mf |= MAP_SHARED;
    return mf;
}

bool Area::isSkipped() const
{
    /* If it's readable, but it's VDSO, it will be dangerous to restore it.
    * In 32-bit mode later Red Hat RHEL Linux 2.6.9 releases use 0xffffe000,
    * the last page of virtual memory.  Note 0xffffe000 >= HIGHEST_VA
    * implies we're in 32-bit mode.
    */
    if (addr >= HIGHEST_VA && addr == (void*)0xffffe000) {
        return true;
    }
#ifdef __x86_64__

    /* And in 64-bit mode later Red Hat RHEL Linux 2.6.9 releases
    * use 0xffffffffff600000 for VDSO.
    */
    if (addr >= HIGHEST_VA && addr == (void*)0xffffffffff600000) {
        return true;
    }
#endif // ifdef __x86_64__

    if (size == 0) {
        /* Kernel won't let us munmap this.  But we don't need to restore it. */
        return true;
    }

    if (0 == strcmp(name, "[vsyscall]") ||
    0 == strcmp(name, "[vectors]") ||
    0 == strcmp(name, "[vvar]") ||
    0 == strcmp(name, "[vdso]")) {
        return true;
    }

    /* Don't save our reserved memory */
    if ((addr == ReservedMemory::getAddr(0)) && (size == ReservedMemory::getSize())) {
        return true;
    }

    /* Don't save area that cannot be promoted to read/write */
    if ((max_prot & (PROT_WRITE|PROT_READ)) != (PROT_WRITE|PROT_READ)) {
        return true;
    }
    
    /* Save area if write permission */
    if (prot & PROT_WRITE) {
        return false;
    }

    /* Save anonymous area even if write protection off, because some
     * games or libs could change protections
     */
    if (flags & Area::AREA_ANON) {
        return false;
    }

    return true;
}

bool Area::isUncommitted(int spmfd) const
{
    if (spmfd == -1)
        return false;
    
    if (prot != PROT_NONE)
        return false;
    
    if (!(flags & AREA_ANON) || !(flags & AREA_PRIV))
        return false;
    
    /* Seek at the beginning of the area pagemap */
    lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(addr) / (4096/8)), SEEK_SET);

    /* Number of pages in the area */
    size_t nb_pages = size / 4096;

    /* Chunk of pagemap values */
    uint64_t pagemaps[512];

    /* Current index in the pagemaps array */
    int pagemap_i = 512;

    for (size_t page_i = 0; page_i < nb_pages; page_i++) {
        /* We read pagemap flags in chunks to avoid too many read syscalls. */
        if ((pagemap_i >= 512)) {
            size_t remaining_pages = (nb_pages-page_i)>512?512:(nb_pages-page_i);
            Utils::readAll(spmfd, pagemaps, remaining_pages*8);
            pagemap_i = 0;
        }
        
        /* Gather the flag for the current pagemap. */
        uint64_t page = pagemaps[pagemap_i++];
        bool page_present = page & (0x1ull << 63);
        if (!page_present)
            return true;
    }
    return false;
}

}
