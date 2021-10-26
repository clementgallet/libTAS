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

#include "vulkanwrappers.h"
#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD.h"
#include "ScreenCapture.h"
#include "frame.h"

#include <string.h>

#define STORE_SYMBOL(str) \
    if (!strcmp(symbol, #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_HOOK | LCF_VULKAN,"  store real function in %p", real_pointer); \
        return real_pointer; \
    }

#define STORE_RETURN_SYMBOL(str) \
    if (!strcmp(symbol, #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_HOOK | LCF_VULKAN,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(libtas::str), real_pointer); \
        return reinterpret_cast<void*>(libtas::str); \
    }

#define STORE_RETURN_SYMBOL_CUSTOM(str) \
    if (!strcmp(reinterpret_cast<const char*>(symbol), #str)) { \
        orig::str = reinterpret_cast<decltype(orig::str)>(real_pointer); \
        debuglogstdio(LCF_HOOK | LCF_VULKAN,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(my##str), real_pointer); \
        return reinterpret_cast<void*>(my##str); \
    }

namespace libtas {

VkPhysicalDevice vk::physicalDevice;
VkDevice vk::device;
VkPhysicalDeviceMemoryProperties vk::deviceMemoryProperties;
VkQueue vk::graphicsQueue;
VkCommandPool vk::commandPool;
VkSwapchainKHR vk::swapchain;
VkFormat vk::colorFormat;
uint32_t vk::swapchainImgIndex;
std::vector<VkImage> vk::swapchainImgs;

uint32_t vk::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < vk::deviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((vk::deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		typeBits >>= 1;
	}

    debuglogstdio(LCF_WINDOW | LCF_VULKAN, "Could not find a suitable memory type");
    return 0;
}

/* For some reason, those functions must not be declared by their original name.
 * This does not matter much, because linking is done with a specific ProcAddr function anyway.
 * So, I'm prefixing them with `my` */
namespace orig { \
    decltype(&myvkCreateDevice) vkCreateDevice; \
    decltype(&myvkDestroyDevice) vkDestroyDevice; \
    decltype(&myvkGetDeviceProcAddr) vkGetDeviceProcAddr; \
}

DEFINE_ORIG_POINTER(vkCreateSwapchainKHR)
DEFINE_ORIG_POINTER(vkQueuePresentKHR)
DEFINE_ORIG_POINTER(vkGetInstanceProcAddr)
DEFINE_ORIG_POINTER(vkAcquireNextImageKHR)
DEFINE_ORIG_POINTER(vkGetPhysicalDeviceMemoryProperties)
DEFINE_ORIG_POINTER(vkCreateImage)
DEFINE_ORIG_POINTER(vkGetImageMemoryRequirements)
DEFINE_ORIG_POINTER(vkAllocateMemory)
DEFINE_ORIG_POINTER(vkBindImageMemory)
DEFINE_ORIG_POINTER(vkCreateCommandPool)
DEFINE_ORIG_POINTER(vkQueueSubmit)
DEFINE_ORIG_POINTER(vkUnmapMemory)
DEFINE_ORIG_POINTER(vkFreeMemory)
DEFINE_ORIG_POINTER(vkDestroyImage)
DEFINE_ORIG_POINTER(vkAllocateCommandBuffers)
DEFINE_ORIG_POINTER(vkBeginCommandBuffer)
DEFINE_ORIG_POINTER(vkCmdPipelineBarrier)
DEFINE_ORIG_POINTER(vkCmdBlitImage)
DEFINE_ORIG_POINTER(vkCmdCopyImage)
DEFINE_ORIG_POINTER(vkEndCommandBuffer)
DEFINE_ORIG_POINTER(vkQueueWaitIdle)
DEFINE_ORIG_POINTER(vkFreeCommandBuffers)
DEFINE_ORIG_POINTER(vkGetImageSubresourceLayout)
DEFINE_ORIG_POINTER(vkMapMemory)
DEFINE_ORIG_POINTER(vkGetSwapchainImagesKHR)

/* If the game uses the vkGetInstanceProcAddr functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* store_orig_and_return_my_symbol(const char* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    STORE_RETURN_SYMBOL(vkCreateSwapchainKHR)
    STORE_RETURN_SYMBOL(vkAcquireNextImageKHR)
    STORE_RETURN_SYMBOL(vkQueuePresentKHR)
    STORE_RETURN_SYMBOL_CUSTOM(vkCreateDevice)
    STORE_RETURN_SYMBOL_CUSTOM(vkDestroyDevice)
    STORE_RETURN_SYMBOL_CUSTOM(vkGetDeviceProcAddr)
    STORE_SYMBOL(vkGetPhysicalDeviceMemoryProperties)
    STORE_SYMBOL(vkCreateImage)
    STORE_SYMBOL(vkGetImageMemoryRequirements)
    STORE_SYMBOL(vkAllocateMemory)
    STORE_SYMBOL(vkBindImageMemory)
    STORE_RETURN_SYMBOL(vkCreateCommandPool)
    STORE_SYMBOL(vkQueueSubmit)
    STORE_SYMBOL(vkUnmapMemory)
    STORE_SYMBOL(vkFreeMemory)
    STORE_SYMBOL(vkDestroyImage)
    STORE_SYMBOL(vkAllocateCommandBuffers)
    STORE_SYMBOL(vkBeginCommandBuffer)
    STORE_SYMBOL(vkCmdPipelineBarrier)
    STORE_SYMBOL(vkCmdBlitImage)
    STORE_SYMBOL(vkCmdCopyImage)
    STORE_SYMBOL(vkEndCommandBuffer)
    STORE_SYMBOL(vkQueueWaitIdle)
    STORE_SYMBOL(vkFreeCommandBuffers)
    STORE_SYMBOL(vkGetImageSubresourceLayout)
    STORE_SYMBOL(vkMapMemory)
    STORE_SYMBOL(vkGetSwapchainImagesKHR)

    return real_pointer;
}

PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    debuglogstdio(LCF_HOOK | LCF_VULKAN, "%s call with symbol %s", __func__, pName);
    LINK_NAMESPACE(vkGetInstanceProcAddr, "vulkan");

    if (!orig::vkGetInstanceProcAddr) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(pName, reinterpret_cast<void*>(orig::vkGetInstanceProcAddr(instance, pName))));
}

PFN_vkVoidFunction myvkGetDeviceProcAddr(VkDevice device, const char* pName)
{
    debuglogstdio(LCF_HOOK | LCF_VULKAN, "%s call with symbol %s", __func__, pName);
    LINK_NAMESPACE(vkGetDeviceProcAddr, "vulkan");

    if (!orig::vkGetDeviceProcAddr) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(pName, reinterpret_cast<void*>(orig::vkGetDeviceProcAddr(device, pName))));
}

VkResult myvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LINK_NAMESPACE(vkCreateDevice, "vulkan");

    if (GlobalState::isNative())
        return orig::vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Store physical device */
    vk::physicalDevice = physicalDevice;

    /* Get memory properties of physical device */
    orig::vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vk::deviceMemoryProperties);

    VkResult res = orig::vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (res == VK_SUCCESS) {
        /* Store the device */
        vk::device = *pDevice;

        /* We are officially using Vulkan now. Initialize screen capture */
        game_info.video |= GameInfo::VULKAN;
        game_info.tosend = true;

        ScreenCapture::fini();
        ScreenCapture::init();
    }
    
    return res;
}

void myvkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    LINK_NAMESPACE(vkDestroyDevice, "vulkan");

    if (GlobalState::isNative())
        return orig::vkDestroyDevice(device, pAllocator);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Stop the screen capture */
    ScreenCapture::fini();

    return orig::vkDestroyDevice(device, pAllocator);
}

VkResult vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    LINK_NAMESPACE(vkCreateCommandPool, "vulkan");

    if (GlobalState::isNative())
        return orig::vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    VkResult res = orig::vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    
    /* Store the command pool */
    vk::commandPool = *pCommandPool;
    
    return res;
}

VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    LINK_NAMESPACE(vkCreateSwapchainKHR, "vulkan");

    if (GlobalState::isNative())
        return orig::vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* We must add support for transfering the swapchain image from/to another image,
     * for encoding and hud. */
    VkSwapchainCreateInfoKHR newCreateInfo = *pCreateInfo;
    newCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    /* Save the color format */
    vk::colorFormat = pCreateInfo->imageFormat;

    return orig::vkCreateSwapchainKHR(device, &newCreateInfo, pAllocator, pSwapchain);
}

VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    LINK_NAMESPACE(vkAcquireNextImageKHR, "vulkan");

    if (GlobalState::isNative())
        return orig::vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    VkResult res = orig::vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    
    debuglogstdio(LCF_WINDOW | LCF_VULKAN, "   obtained index %d", *pImageIndex);

    /* Store swapchain and image index */
    vk::swapchain = swapchain;
    vk::swapchainImgIndex = *pImageIndex;
    return res;
}

VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    LINK_NAMESPACE(vkQueuePresentKHR, "vulkan");
    LINK_NAMESPACE(vkGetSwapchainImagesKHR, "vulkan");

    if (GlobalState::isNative())
        return orig::vkQueuePresentKHR(queue, pPresentInfo);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Store the graphic queue */
    vk::graphicsQueue = queue;

    /* We check that only one swapchain is being presented.
     * Otherwise, I don't know what to do. */
    if (pPresentInfo->swapchainCount != 1) {
        debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_WARNING, "Multiple swapchains are being presented");
    }

    /* Get the currently acquired swapchain image
     * TODO: we don't need to get this on every call. */
    uint32_t count;
    orig::vkGetSwapchainImagesKHR(vk::device, vk::swapchain, &count, nullptr);
    vk::swapchainImgs.resize(count);
    orig::vkGetSwapchainImagesKHR(vk::device, vk::swapchain, &count, vk::swapchainImgs.data());

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD renderHUD;
    frameBoundary([&] () {
        static bool first = true;
        
        if (first) {
            first = false;
            orig::vkQueuePresentKHR(queue, pPresentInfo);    
        }
        else {
            VkPresentInfoKHR pi = *pPresentInfo;

            /* After the first time, don't wait on any semaphore because rendering
             * is already completed. */
            pi.waitSemaphoreCount = 0;
            pi.pWaitSemaphores = nullptr;

            /* Present the queue with the stored image index, because we will 
             * acquire other images when presenting again. */
            pi.pImageIndices = &vk::swapchainImgIndex;
            debuglogstdio(LCF_WINDOW | LCF_VULKAN, "vkQueuePresentKHR called again with image index %d", vk::swapchainImgIndex);
            orig::vkQueuePresentKHR(queue, &pi);
        }
    }, renderHUD);
#else
    frameBoundary([&] () {orig::vkQueuePresentKHR(queue, pPresentInfo);});
#endif

    return VK_SUCCESS;
}

}
