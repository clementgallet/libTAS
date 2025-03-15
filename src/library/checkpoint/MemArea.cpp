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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "MemArea.h"
#include "ReservedMemory.h"

#include "fileio/FileHandleList.h"
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
    /* Savefiles must be saved */
    if (flags & Area::AREA_SAVEFILE) {
        return false;
    }

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
    
    if (flags & Area::AREA_MEMFD) {
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
    
    /* The logic here is that if a private anon area has no permission (read or
     * write), and has at least one uncommitted page, then all pages are uncommitted.
     * I don't remember exactly how I came to this conclusion, it has to do with
     * the VM_ACCOUNT flag that is set when an area becomes writable, and the 
     * fact that two areas without matching VM_ACCOUNT flag cannot be merged. */
    
    /* Seek at the beginning of the area pagemap */
    lseek(spmfd, static_cast<off_t>(reinterpret_cast<uintptr_t>(addr) / (4096/8)), SEEK_SET);
    uint64_t page;
    Utils::readAll(spmfd, &page, 8);
    bool page_present = page & (0x1ull << 63);
    return !page_present;
}

void Area::fillDeletedFd()
{
    /* fd may have already been filled in the case of savefiles */
    if (fd != -1)
        return;
    
    if (!(flags & Area::AREA_FILE))
        return;
    
    /* If the file is deleted, look at our stored file handles for a matching
     * file name */
    /* FIXME: Different files may have the same filename! e.g. in the case when 
     * memfd_create is used, the filename is only indicative, and is not
     * necessarily unique, oops! Maybe use file inode and something like that? */
    if (strlen(name) > 10 && (0 == strcmp(name + strlen(name) - 10, " (deleted)"))) {
        fd = FileHandleList::fdFromFile(name);
    }
}

}
