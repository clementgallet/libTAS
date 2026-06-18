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

#ifndef VK_SKIP_PROC_ADDR_PROCS
VK_PROC(GetInstanceProcAddr)
VK_PROC(GetDeviceProcAddr)
#endif

VK_PROC(CreateInstance)
VK_PROC(EnumerateDeviceExtensionProperties)
VK_PROC(GetPhysicalDeviceSurfacePresentModesKHR)
VK_PROC(CreateDevice)
VK_PROC(DestroyDevice)
VK_PROC(CreateSwapchainKHR)
VK_PROC(QueuePresentKHR)
VK_PROC(AcquireNextImageKHR)
VK_PROC(GetPhysicalDeviceMemoryProperties)
VK_PROC(CreateImage)
VK_PROC(GetImageMemoryRequirements)
VK_PROC(AllocateMemory)
VK_PROC(BindImageMemory)
VK_PROC(CreateCommandPool)
VK_PROC(CreateDescriptorPool)
VK_PROC(DestroyDescriptorPool)
VK_PROC(ResetCommandPool)
VK_PROC(CreateRenderPass)
VK_PROC(CmdBeginRenderPass)
VK_PROC(CmdEndRenderPass)
VK_PROC(GetDeviceQueue)
VK_PROC(QueueSubmit)
VK_PROC(UnmapMemory)
VK_PROC(FreeMemory)
VK_PROC(DestroyImage)
VK_PROC(AllocateCommandBuffers)
VK_PROC(BeginCommandBuffer)
VK_PROC(CmdPipelineBarrier)
VK_PROC(CmdBlitImage)
VK_PROC(CmdCopyImage)
VK_PROC(EndCommandBuffer)
VK_PROC(QueueWaitIdle)
VK_PROC(DeviceWaitIdle)
VK_PROC(ResetFences)
VK_PROC(WaitForFences)
VK_PROC(FreeCommandBuffers)
VK_PROC(GetImageSubresourceLayout)
VK_PROC(MapMemory)
VK_PROC(GetSwapchainImagesKHR)
VK_PROC(CreateImageView)
VK_PROC(CreateSampler)
VK_PROC(CreateFramebuffer)
VK_PROC(CreateFence)
VK_PROC(CreateSemaphore)
VK_PROC(DestroyRenderPass)
VK_PROC(DestroySemaphore)
VK_PROC(DestroyFence)
VK_PROC(DestroyCommandPool)
VK_PROC(DestroyImageView)
VK_PROC(DestroySampler)
VK_PROC(DestroyFramebuffer)
VK_PROC(DestroySwapchainKHR)
VK_PROC(CmdClearColorImage)
VK_PROC(CmdDraw)
VK_PROC(CmdDrawIndirect)
// VK_PROC(CmdDrawIndirectCount)
VK_PROC(CmdDrawIndexed)
VK_PROC(CmdDrawIndexedIndirect)
// VK_PROC(CmdDrawIndexedIndirectCount)
