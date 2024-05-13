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

#ifndef LIBTAS_RESERVEDMEMORY_H
#define LIBTAS_RESERVEDMEMORY_H

#include "StateHeader.h"

#include <cstdint> // intptr_t
#include <cstddef> // size_t

#define ONE_MB 1024 * 1024

namespace libtas {
namespace ReservedMemory {
    enum Sizes {
        COMPRESSED_SIZE = 4 * ONE_MB,
        STACK_SIZE = 5 * ONE_MB,
        PAGEMAPS_SIZE = 11*sizeof(int),
        PAGES_SIZE = 11*sizeof(int),
        SS_SLOTS_SIZE = 11*sizeof(bool),
        SH_SIZE = sizeof(StateHeader),
    };
    enum Addresses {
        COMPRESSED_ADDR = 0,
        STACK_ADDR = COMPRESSED_ADDR + COMPRESSED_SIZE,
        PAGEMAPS_ADDR = STACK_ADDR + STACK_SIZE,
        PAGES_ADDR = PAGEMAPS_ADDR + PAGEMAPS_SIZE,
        SS_SLOTS_ADDR = PAGES_ADDR + PAGES_SIZE,
        SH_ADDR = SS_SLOTS_ADDR + SS_SLOTS_SIZE,
        RESTORE_TOTAL_SIZE = SH_ADDR + SH_SIZE,
    };

    void init();
    void* getAddr(intptr_t offset);
    size_t getSize();
}
}

#endif
