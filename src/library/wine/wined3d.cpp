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

#include "wined3d.h"
#include "../hookpatch.h"
#include "../logging.h"
// #include <sys/mman.h>

namespace libtas {

namespace orig {

static void __attribute__((noinline)) wined3d_device_update_sub_resource(void *device, void *resource,
        unsigned int sub_resource_idx, const void *box, const void *data, unsigned int row_pitch,
        unsigned int depth_pitch, unsigned int flags)
{
    static long x__ = 0;
    x__++;
    x__++;
    if (x__==2) {
        debuglog(LCF_HOOK | LCF_ERROR, "Function got called before it was set up!");
    }
    x__++;
    x__++;
}

}

void wined3d_device_update_sub_resource(void *device, void *resource,
        unsigned int sub_resource_idx, const void *box, const void *data, unsigned int row_pitch,
        unsigned int depth_pitch, unsigned int flags)
{
    DEBUGLOGCALL(LCF_WINE);
    return orig::wined3d_device_update_sub_resource(device, resource, sub_resource_idx, box, data, row_pitch, depth_pitch, flags);
}

void hook_wined3d()
{
    HOOK_PATCH_ORIG(wined3d_device_update_sub_resource, "wined3d.dll.so");
}


}
