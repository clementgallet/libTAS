/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "kernel32.h"
#include "winehook.h"
#include "../hookpatch.h"
#include "../logging.h"

namespace libtas {

namespace orig {

static int __stdcall __attribute__((noinline)) WaitForMultipleObjectsEx( int count, const void **handles,
                                           bool wait_all, int timeout,
                                           bool alertable )
{
    static long x__ = 0;
    x__++;
    x__++;
    if (x__==2) {
        debuglog(LCF_HOOK | LCF_ERROR, "Function got called before it was set up!");
    }
    x__++;
    x__++;
    return 0;
}

}

int __stdcall WaitForMultipleObjectsEx( int count, const void **handles,
                                       bool wait_all, int timeout,
                                       bool alertable )
{
    DEBUGLOGCALL(LCF_WINE);
    return orig::WaitForMultipleObjectsEx(count, handles, wait_all, timeout, alertable);
}

void hook_kernel32()
{
    // HOOK_PATCH_ORIG(WaitForMultipleObjectsEx, "kernel32.dll.so");
}


}
