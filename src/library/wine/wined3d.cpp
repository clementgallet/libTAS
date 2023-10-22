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

#include "wined3d.h"
#include "winehook.h"
#include "../hookpatch.h"
#include "../logging.h"
#include "../checkpoint/ThreadSync.h"
#include "../global.h"

namespace libtas {

namespace orig {

static void* __stdcall __attribute__((noinline)) wined3d_texture_get_resource(void *texture)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static unsigned long __attribute__((noinline)) wined3d_swapchain_present(void *swapchain, const void *src_rect,
        const void *dst_rect, void *dst_window_override, unsigned int swap_interval, unsigned int flags)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static unsigned long __attribute__((noinline)) wined3d_resource_map(void *resource, unsigned int sub_resource_idx,
        void *map_desc, const void *box, unsigned int flags)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

}

void* __stdcall wined3d_texture_get_resource(void *texture)
{
    DEBUGLOGCALL(LCF_WINE);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_WITNESS) {
        ThreadSync::detSignal(true);
    }
    return orig::wined3d_texture_get_resource(texture);
}

unsigned long wined3d_swapchain_present(void *swapchain, const void *src_rect,
        const void *dst_rect, void *dst_window_override, unsigned int swap_interval, unsigned int flags)
{
    DEBUGLOGCALL(LCF_WINE);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_WITNESS) {
        ThreadSync::detInit();
    }
    return orig::wined3d_swapchain_present(swapchain, src_rect, dst_rect, dst_window_override, swap_interval, flags);
}

unsigned long wined3d_resource_map(void *resource, unsigned int sub_resource_idx,
        void *map_desc, const void *box, unsigned int flags)
{
    DEBUGLOGCALL(LCF_WINE);
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_WITNESS) {
        ThreadSync::detSignal(true);
    }
    return orig::wined3d_resource_map(resource, sub_resource_idx, map_desc, box, flags);
}

void hook_wined3d()
{
    if (Global::shared_config.game_specific_sync & SharedConfig::GC_SYNC_WITNESS) {
        HOOK_PATCH_ORIG(wined3d_texture_get_resource, "wined3d.dll.so");
        HOOK_PATCH_ORIG(wined3d_swapchain_present, "wined3d.dll.so");
        HOOK_PATCH_ORIG(wined3d_resource_map, "wined3d.dll.so");
    }
}


}
