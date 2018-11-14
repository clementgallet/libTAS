/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ReservedMemory.h"
#include "../logging.h"
#include <string.h>
#include <sys/mman.h>

namespace libtas {

static intptr_t restoreAddr = 0;
static size_t restoreLength = 0;

void ReservedMemory::init()
{
    /* Create a special place to hold restore memory.
     * will be used for the second stack we will switch to, as well as
     * the ProcSelfMaps object that need some space.
     */
    if (restoreAddr == 0) {
        restoreLength = RESTORE_TOTAL_SIZE;
        void* addr = mmap(nullptr, restoreLength + (2 * 4096), PROT_NONE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        MYASSERT(addr != MAP_FAILED)
        restoreAddr = reinterpret_cast<intptr_t>(addr) + 4096;
        MYASSERT(mprotect(reinterpret_cast<void*>(restoreAddr), restoreLength, PROT_READ | PROT_WRITE) == 0)
        memset(reinterpret_cast<void*>(restoreAddr), 0, restoreLength);
        // debuglogstdio(LCF_ERROR, "Setup reserved space from %p to %p", reinterpret_cast<void*>(restoreAddr+ONE_MB), reinterpret_cast<void*>(restoreAddr+restoreLength));
    }
}

void* ReservedMemory::getAddr(intptr_t offset)
{
    return reinterpret_cast<void*>(restoreAddr+offset);
}

size_t ReservedMemory::getSize()
{
    return restoreLength;
}

}
