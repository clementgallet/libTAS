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

#include "logging.h"

#include <sys/mman.h> // PROT_READ, PROT_WRITE, etc.
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/vm_prot.h> // VM_PROT_READ, VM_PROT_WRITE, etc.
#endif

namespace libtas {

void Area::print(const char* prefix) const
{
#ifdef __unix__
    debuglogstdio(LCF_CHECKPOINT, "%s Region %c%c%c%c %p-%p (%s) with size %zu and flags %x",
    prefix,
    (prot&PROT_READ)?'r':'-', (prot&PROT_WRITE)?'w':'-', (prot&PROT_EXEC)?'x':'-', (flags&AREA_SHARED)?'s':'p',
    addr, endAddr, name, size, flags);
#elif defined(__APPLE__) && defined(__MACH__)
    debuglogstdio(LCF_CHECKPOINT, "%s Region %c%c%c%c %p-%p (%s) with size %zu and flags %x",
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

}
