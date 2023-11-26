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

#include <cstdint> // intptr_t
#include <cstddef> // size_t

#define ONE_MB 1024 * 1024
#define RESTORE_TOTAL_SIZE 5 * ONE_MB

namespace libtas {
namespace ReservedMemory {
    enum Addresses {
        PAGEMAPS_ADDR = 0,
        PAGES_ADDR = 11*sizeof(int),
        SS_SLOTS_ADDR = 22*sizeof(int),
        PSM_ADDR = 22*sizeof(int)+11*sizeof(bool),
        STACK_ADDR = ONE_MB,
    };
    enum Sizes {
        PAGEMAPS_SIZE = PAGES_ADDR - PAGEMAPS_ADDR,
        PAGES_SIZE = SS_SLOTS_ADDR - PAGES_ADDR,
        SS_SLOTS_SIZE = PSM_ADDR - SS_SLOTS_ADDR,
        PSM_SIZE = STACK_ADDR - PSM_ADDR,
        STACK_SIZE = RESTORE_TOTAL_SIZE - STACK_ADDR,
    };

    void init();
    void* getAddr(intptr_t offset);
    size_t getSize();


}
}

#endif
