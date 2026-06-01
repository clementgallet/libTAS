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

#include "../external/vulkan_core.h"

namespace libtas {

struct VKProcs vkProcs;

#define GET_VK_POINTER(FUNC) \
    vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(proc("vk" #FUNC)); \
    if (!vkProcs.FUNC) \
        link_function((void**)&vkProcs.FUNC, "vk" #FUNC, "libvulkan.so");

void vk_load_procs(VKGetProcAddressProc proc)
{
    static VKGetProcAddressProc old_proc = nullptr;
    if (proc == old_proc)
        return;
    old_proc = proc;
    
    GET_VK_POINTER(CreateSwapchainKHR)
    GET_VK_POINTER(QueuePresentKHR)
    GET_VK_POINTER(AcquireNextImageKHR)
    GET_VK_POINTER(GetPhysicalDeviceMemoryProperties)
    GET_VK_POINTER(CreateImage)
    GET_VK_POINTER(GetImageMemoryRequirements)
    GET_VK_POINTER(AllocateMemory)
    GET_VK_POINTER(BindImageMemory)
    GET_VK_POINTER(CreateCommandPool)
    GET_VK_POINTER(CreateDescriptorPool)
    GET_VK_POINTER(DestroyDescriptorPool)
    GET_VK_POINTER(CreateRenderPass)
    GET_VK_POINTER(CmdBeginRenderPass)
    GET_VK_POINTER(CmdEndRenderPass)
    GET_VK_POINTER(GetDeviceQueue)
    GET_VK_POINTER(QueueSubmit)
    GET_VK_POINTER(UnmapMemory)
    GET_VK_POINTER(FreeMemory)
    GET_VK_POINTER(DestroyImage)
    GET_VK_POINTER(AllocateCommandBuffers)
    GET_VK_POINTER(BeginCommandBuffer)
    GET_VK_POINTER(CmdPipelineBarrier)
    GET_VK_POINTER(CmdBlitImage)
    GET_VK_POINTER(CmdCopyImage)
    GET_VK_POINTER(EndCommandBuffer)
    GET_VK_POINTER(QueueWaitIdle)
    GET_VK_POINTER(FreeCommandBuffers)
    GET_VK_POINTER(GetImageSubresourceLayout)
    GET_VK_POINTER(MapMemory)
    GET_VK_POINTER(GetSwapchainImagesKHR)
    GET_VK_POINTER(CreateImageView)
    GET_VK_POINTER(CreateSampler)
    GET_VK_POINTER(CreateFramebuffer)
    GET_VK_POINTER(CreateFence)
    GET_VK_POINTER(CreateSemaphore)
    GET_VK_POINTER(DestroyRenderPass)
    GET_VK_POINTER(DestroySemaphore)
    GET_VK_POINTER(DestroyFence)
    GET_VK_POINTER(DestroyCommandPool)
    GET_VK_POINTER(DestroyImageView)
    GET_VK_POINTER(DestroySampler)
    GET_VK_POINTER(DestroyFramebuffer)
    GET_VK_POINTER(DestroySwapchainKHR)
    GET_VK_POINTER(CmdClearColorImage)
}

}
