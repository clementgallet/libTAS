/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "mallocwrappers.h"

#include "logging.h"
#include "GlobalState.h"

#include <cstdlib>

namespace libtas {

void *malloc (size_t size) __THROW
{
    /* Some custom implementation of malloc can call a function that we hook, 
     * and our function ultimately calls malloc, which creates a deadlock.
     * Example: Antichamber (Unreal) uses tcmalloc, and has the following softlock:
     * - malloc() -> libtas::getpid() -> backtrace_symbols() -> malloc()
     * Marking the call as native can mitigate the deadlock. */
    GlobalNative gn;
    return calloc(1, size);
}

}
