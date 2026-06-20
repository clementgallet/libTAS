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

#ifndef LIBTAS_VULKANLOADER_H_INCL
#define LIBTAS_VULKANLOADER_H_INCL

#include "hook.h"

#include "../external/vulkan_core.h"

namespace libtas {

/* vk internal state */
struct VKProcs {
#define VK_PROC(FUNC) decltype(&vk##FUNC) FUNC;
#include "vulkanprocs.h"
#undef VK_PROC
};

typedef void (*VKProc)(void);
typedef VKProc (*VKGetProcAddressProc)(const char *proc);

void vk_load_procs(VKGetProcAddressProc proc);

extern struct VKProcs vkProcs;
extern struct VKProcs myvkProcs;

void* vk_find_my_proc(const char *name);

#define LINK_VK_POINTER(FUNC) \
    if (!vkProcs.FUNC) \
        link_function((void**)&vkProcs.FUNC, "vk" #FUNC, "libvulkan.so.1");

void vk_store_proc(const char *name, void* proc);

}

#endif
