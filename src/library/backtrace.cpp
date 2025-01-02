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

#include "backtrace.h"

#include <execinfo.h>
#include <unistd.h>

namespace libtas {

void printBacktrace(void)
{
    thread_local static int recurs = 0;
    if (recurs)
        return;
    recurs = 1;

    void* addresses[256];
    const int n = backtrace(addresses, 256);
    /* Use `backtrace_symbols_fd` instead of `backtrace_symbols` to avoid using
     * `malloc`. Also, don't print the top three functions as those are libtas
     * functions calling this */
    backtrace_symbols_fd(addresses+3, n-3, STDERR_FILENO);
    recurs = 0;
}

}
