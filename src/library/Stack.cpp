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
}

}
