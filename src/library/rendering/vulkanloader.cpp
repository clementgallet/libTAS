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

#include "vulkanloader.h"

#include "hook.h"
#include "logging.h"
#include "GlobalState.h"

#include "../external/vulkan_core.h"

#include <dlfcn.h>

namespace libtas {

struct VKProcs vkProcs;

void vk_load_procs(VKGetProcAddressProc proc)
{
    static VKGetProcAddressProc old_proc = nullptr;
    if (proc == old_proc)
        return;
    old_proc = proc;
    
#define VK_PROC(FUNC) \
    if (!vkProcs.FUNC) \
        vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(proc("vk" #FUNC)); \
    if (!vkProcs.FUNC) { \
        GlobalNative gn; \
        void* handle = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_DEEPBIND); \
        if (handle != NULL) { \
            vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(dlsym(handle, "vk" #FUNC)); \
            dlclose(handle); \
        } \
    }

/* We don't load the different Get*ProcAddr functions, because if they were not loaded already, it means
 * that the game is linking directly to each vk function. In that case, those functions will not work. */
#define VK_SKIP_PROC_ADDR_PROCS
#include "vulkanprocs.h"
#undef VK_SKIP_PROC_ADDR_PROCS
#undef VK_PROC
}

void vk_store_proc(const char *name, void* proc)
{
#define VK_PROC(FUNC) \
    if (!strcmp(name, "vk" #FUNC)) { \
        vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(proc); \
        LOG(LL_DEBUG, LCF_VULKAN,"  Storing Vulkan function %s at address %p", "vk" #FUNC, proc); \
        return; \
    }

#include "vulkanprocs.h"
#undef VK_PROC
}

}
