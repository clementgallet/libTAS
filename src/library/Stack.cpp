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
 */

#include "Stack.h"
#include "logging.h"
#include "checkpoint/ProcSelfMaps.h"
#include "checkpoint/MemArea.h"

#include <sys/resource.h>
#include <alloca.h>

namespace libtas {

void Stack::grow()
{
    /* Most of the code taken from <http://dmtcp.sourceforge.net> */

    /* Grow the stack to the stack limit */
    struct rlimit rlim;
    size_t stackSize;

    MYASSERT(getrlimit(RLIMIT_STACK, &rlim) == 0);
    if (rlim.rlim_cur == RLIM_INFINITY) {
        stackSize = 8 * 1024 * 1024;
        if (rlim.rlim_max != RLIM_INFINITY) {
            stackSize = (rlim.rlim_max<stackSize)?rlim.rlim_max:stackSize;
        }
    } else {
      stackSize = rlim.rlim_cur;
    }

#ifdef __unix__
    /* Find the current stack area */
    ProcSelfMaps procSelfMaps;
    Area stackArea;
    uintptr_t stackPointer = reinterpret_cast<uintptr_t>(&stackArea);
    while (procSelfMaps.getNextArea(&stackArea)) {
        if ((stackPointer >= reinterpret_cast<uintptr_t>(stackArea.addr)) && (stackPointer < reinterpret_cast<uintptr_t>(stackArea.endAddr))) {
            break;
        }
    }

    /* Check if we found the stack */
    if (!stackArea.addr) {
        LOG(LL_ERROR, LCF_CHECKPOINT, "Could not find the stack area");
        return;
    }

    // LOG(LL_INFO, LCF_NONE, "Current stack size is %d", stackArea.size);

    /* Check if we need to grow it */
    if (stackSize <= stackArea.size)
        return;

    /* Grow the stack */
    size_t allocSize = stackSize - stackArea.size - 4095;
    void* tmpbuf = alloca(allocSize);
    MYASSERT(tmpbuf != nullptr);
    memset(tmpbuf, 0, allocSize);
    LOG(LL_DEBUG, LCF_NONE, "Some value %d", static_cast<char*>(tmpbuf)[0]);

    /* Look at the new stack area */
    /* Apparently, if we don't use another local variable here, the compiler
     * optimizes the alloca code above! */
    ProcSelfMaps newProcSelfMaps;
    while (newProcSelfMaps.getNextArea(&stackArea)) {
        if ((stackPointer >= reinterpret_cast<uintptr_t>(stackArea.addr)) && (stackPointer < reinterpret_cast<uintptr_t>(stackArea.endAddr))) {
            break;
        }
    }

    /* Check if we found the stack */
    if (stackArea.addr) {
        // LOG(LL_INFO, LCF_NONE, "New stack size is %d", stackArea.size);
        return;
    }
#endif

}

}
