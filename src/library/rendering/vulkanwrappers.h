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

#ifndef LIBTAS_VULKANWRAPPERS_H_INCL
#define LIBTAS_VULKANWRAPPERS_H_INCL

#include "hook.h"
#include "../external/vulkan_core.h"

#include <cstddef>
#include <vector>

namespace libtas {


/* Structs taken from ImGui_ImplVulkanH_* versions */
struct Vulkan_Frame
{
    VkCommandPool       commandPool;
    VkCommandBuffer     screenCommandBuffer;
    VkCommandBuffer     clearCommandBuffer;
    VkCommandBuffer     osdCommandBuffer;
    VkFence             fence;
    VkImage             backbuffer;
    VkImageView         backbufferView;
    VkFramebuffer       framebuffer;
};

struct Vulkan_FrameSemaphores
{
    VkSemaphore         imageAcquiredSemaphore;
    VkSemaphore         screenCompleteSemaphore;
    VkSemaphore         clearCompleteSemaphore;
    VkSemaphore         osdCompleteSemaphore;
};

struct Vulkan_Context
{
    VkAllocationCallbacks* allocator;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkQueue graphicsQueue;
    uint32_t queueFamily;
    VkDescriptorPool descriptorPool;
    VkFormat colorFormat;
    int                 width;
    int                 height;
    VkSwapchainKHR      swapchain;
    VkRenderPass        renderPass;
    VkClearValue        clearValue;
    uint32_t            frameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    uint32_t            imageCount;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
    uint32_t            semaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
    VkSemaphore         currentSemaphore;
    std::vector<Vulkan_Frame> frames;
    std::vector<Vulkan_FrameSemaphores> frameSemaphores;
};

/* Export several variables used for rendering */
namespace vk {
    extern Vulkan_Context context;

    void checkVkResult(VkResult err);

    /* Helper function for getting memory type */
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);    
}

OVERRIDE PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName);

VkResult myvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    
PFN_vkVoidFunction myvkGetDeviceProcAddr(VkDevice device, const char* pName);

VkResult myvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);

void myvkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);

void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);

VkResult vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);

VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

void vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    
VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

void vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
void vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
void vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
void vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
void vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
void vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

}

#endif
