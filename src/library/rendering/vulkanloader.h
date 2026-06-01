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

#define DEFINE_VK_POINTER(FUNC) \
    decltype(&vk##FUNC) FUNC;

namespace libtas {

/* vk internal state */
struct VKProcs {
    DEFINE_VK_POINTER(CreateInstance)
    DEFINE_VK_POINTER(CreateDevice)
    DEFINE_VK_POINTER(DestroyDevice)
    DEFINE_VK_POINTER(CreateSwapchainKHR)
    DEFINE_VK_POINTER(QueuePresentKHR)
    DEFINE_VK_POINTER(GetInstanceProcAddr)
    DEFINE_VK_POINTER(GetDeviceProcAddr)
    DEFINE_VK_POINTER(AcquireNextImageKHR)
    DEFINE_VK_POINTER(GetPhysicalDeviceMemoryProperties)
    DEFINE_VK_POINTER(CreateImage)
    DEFINE_VK_POINTER(GetImageMemoryRequirements)
    DEFINE_VK_POINTER(AllocateMemory)
    DEFINE_VK_POINTER(BindImageMemory)
    DEFINE_VK_POINTER(CreateCommandPool)
    DEFINE_VK_POINTER(CreateDescriptorPool)
    DEFINE_VK_POINTER(DestroyDescriptorPool)
    DEFINE_VK_POINTER(CreateRenderPass)
    DEFINE_VK_POINTER(CmdBeginRenderPass)
    DEFINE_VK_POINTER(CmdEndRenderPass)
    DEFINE_VK_POINTER(GetDeviceQueue)
    DEFINE_VK_POINTER(QueueSubmit)
    DEFINE_VK_POINTER(UnmapMemory)
    DEFINE_VK_POINTER(FreeMemory)
    DEFINE_VK_POINTER(DestroyImage)
    DEFINE_VK_POINTER(AllocateCommandBuffers)
    DEFINE_VK_POINTER(BeginCommandBuffer)
    DEFINE_VK_POINTER(CmdPipelineBarrier)
    DEFINE_VK_POINTER(CmdBlitImage)
    DEFINE_VK_POINTER(CmdCopyImage)
    DEFINE_VK_POINTER(EndCommandBuffer)
    DEFINE_VK_POINTER(QueueWaitIdle)
    DEFINE_VK_POINTER(FreeCommandBuffers)
    DEFINE_VK_POINTER(GetImageSubresourceLayout)
    DEFINE_VK_POINTER(MapMemory)
    DEFINE_VK_POINTER(GetSwapchainImagesKHR)
    DEFINE_VK_POINTER(CreateImageView)
    DEFINE_VK_POINTER(CreateSampler)
    DEFINE_VK_POINTER(CreateFramebuffer)
    DEFINE_VK_POINTER(CreateFence)
    DEFINE_VK_POINTER(CreateSemaphore)
    DEFINE_VK_POINTER(DestroyRenderPass)
    DEFINE_VK_POINTER(DestroySemaphore)
    DEFINE_VK_POINTER(DestroyFence)
    DEFINE_VK_POINTER(DestroyCommandPool)
    DEFINE_VK_POINTER(DestroyImageView)
    DEFINE_VK_POINTER(DestroySampler)
    DEFINE_VK_POINTER(DestroyFramebuffer)
    DEFINE_VK_POINTER(DestroySwapchainKHR)
    DEFINE_VK_POINTER(CmdClearColorImage)
    DEFINE_VK_POINTER(CmdDraw)
    DEFINE_VK_POINTER(CmdDrawIndirect)
    DEFINE_VK_POINTER(CmdDrawIndirectCount)
    DEFINE_VK_POINTER(CmdDrawIndexed)
    DEFINE_VK_POINTER(CmdDrawIndexedIndirect)
    DEFINE_VK_POINTER(CmdDrawIndexedIndirectCount)
};

typedef void (*VKProc)(void);
typedef VKProc (*VKGetProcAddressProc)(const char *proc);

void vk_load_procs(VKGetProcAddressProc proc);

extern struct VKProcs vkProcs;

#define LINK_VK_POINTER(FUNC) \
    if (!vkProcs.FUNC) \
        link_function((void**)&vkProcs.FUNC, "vk" #FUNC, "libvulkan.so");

}

#endif
