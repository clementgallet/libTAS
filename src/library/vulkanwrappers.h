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

#ifndef LIBTAS_VULKANWRAPPERS_H_INCL
#define LIBTAS_VULKANWRAPPERS_H_INCL

#include "global.h"
#include <cstddef>
#include <vector>
#include "../external/vulkan_core.h"

namespace libtas {

/* Export several variables used for rendering */
namespace vk {
    extern VkPhysicalDevice physicalDevice;
    extern VkDevice device;
    extern VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    extern bool supportsBlit;
    extern VkQueue graphicsQueue;
    extern VkCommandPool commandPool;
    extern VkSwapchainKHR swapchain;
    extern VkFormat colorFormat;
    extern uint32_t swapchainImgIndex;
    extern std::vector<VkImage> swapchainImgs;

    /* Helper function for getting memory type */
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);    
}

OVERRIDE PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName);

OVERRIDE PFN_vkVoidFunction myvkGetDeviceProcAddr(VkDevice device, const char* pName);

VkResult myvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);

void myvkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);


VkResult vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);

VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

}

#endif
