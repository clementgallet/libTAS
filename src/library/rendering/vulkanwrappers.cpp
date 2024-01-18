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

#include "vulkanwrappers.h"

#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_Vulkan.h"
#include "screencapture/ScreenCapture.h"
#include "frame.h"
#include "global.h"
#include "GlobalState.h"

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

Vulkan_Context vk::context = {};

#define VKCHECKERROR(err) \
do { if (err < 0) debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "Vulkan error: %d", err); } while (0)

void vk::checkVkResult(VkResult err)
{
    if (err == 0)
        return;
    debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "Vulkan error: %d", err);
}

uint32_t vk::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < vk::context.deviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((vk::context.deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
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
    decltype(&myvkCreateInstance) vkCreateInstance; \
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
DEFINE_ORIG_POINTER(vkCreateDescriptorPool)
DEFINE_ORIG_POINTER(vkCreateRenderPass)
DEFINE_ORIG_POINTER(vkCmdBeginRenderPass)
DEFINE_ORIG_POINTER(vkCmdEndRenderPass)
DEFINE_ORIG_POINTER(vkGetDeviceQueue)
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
DEFINE_ORIG_POINTER(vkCreateImageView)
DEFINE_ORIG_POINTER(vkCreateSampler)
DEFINE_ORIG_POINTER(vkCreateFramebuffer)
DEFINE_ORIG_POINTER(vkCreateFence)
DEFINE_ORIG_POINTER(vkCreateSemaphore)
DEFINE_ORIG_POINTER(vkDestroyRenderPass)
DEFINE_ORIG_POINTER(vkDestroySemaphore)
DEFINE_ORIG_POINTER(vkDestroyFence)
DEFINE_ORIG_POINTER(vkDestroyCommandPool)
DEFINE_ORIG_POINTER(vkDestroyImageView)
DEFINE_ORIG_POINTER(vkDestroySampler)
DEFINE_ORIG_POINTER(vkDestroyFramebuffer)
DEFINE_ORIG_POINTER(vkDestroySwapchainKHR)
DEFINE_ORIG_POINTER(vkCmdClearColorImage)


#define VKFUNCSKIPDRAW(NAME, DECL, ARGS) \
DEFINE_ORIG_POINTER(NAME)\
void NAME DECL\
{\
    DEBUGLOGCALL(LCF_VULKAN);\
    if (!Global::skipping_draw)\
        return orig::NAME ARGS;\
}

VKFUNCSKIPDRAW(vkCmdDraw, (VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance), (commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance))
VKFUNCSKIPDRAW(vkCmdDrawIndirect, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride), (commandBuffer, buffer, offset, drawCount, stride))
VKFUNCSKIPDRAW(vkCmdDrawIndirectCount, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride), (commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride))
VKFUNCSKIPDRAW(vkCmdDrawIndexed, (VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance), (commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance))
VKFUNCSKIPDRAW(vkCmdDrawIndexedIndirect, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride), (commandBuffer, buffer, offset, drawCount, stride))
VKFUNCSKIPDRAW(vkCmdDrawIndexedIndirectCount, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride), (commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride))

/* If the game uses the vkGetInstanceProcAddr functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* store_orig_and_return_my_symbol(const char* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    STORE_RETURN_SYMBOL_CUSTOM(vkCreateInstance)
    STORE_RETURN_SYMBOL(vkCreateSwapchainKHR)
    STORE_RETURN_SYMBOL(vkAcquireNextImageKHR)
    STORE_RETURN_SYMBOL(vkQueuePresentKHR)
    STORE_SYMBOL(vkCreateImageView)
    STORE_SYMBOL(vkCreateSampler)
    STORE_SYMBOL(vkCreateFramebuffer)    
    STORE_RETURN_SYMBOL_CUSTOM(vkCreateDevice)
    STORE_RETURN_SYMBOL_CUSTOM(vkDestroyDevice)
    STORE_RETURN_SYMBOL_CUSTOM(vkGetDeviceProcAddr)
    STORE_SYMBOL(vkGetPhysicalDeviceMemoryProperties)
    STORE_SYMBOL(vkCreateImage)
    STORE_SYMBOL(vkGetImageMemoryRequirements)
    STORE_SYMBOL(vkAllocateMemory)
    STORE_SYMBOL(vkBindImageMemory)
    STORE_SYMBOL(vkCreateCommandPool)
    STORE_RETURN_SYMBOL(vkCreateDescriptorPool)
    STORE_SYMBOL(vkCreateRenderPass)
    STORE_RETURN_SYMBOL(vkGetDeviceQueue)
    STORE_RETURN_SYMBOL(vkDestroySwapchainKHR)
    STORE_SYMBOL(vkCmdBeginRenderPass)
    STORE_SYMBOL(vkQueueSubmit)
    STORE_SYMBOL(vkUnmapMemory)
    STORE_SYMBOL(vkFreeMemory)
    STORE_SYMBOL(vkDestroyImage)
    STORE_SYMBOL(vkAllocateCommandBuffers)
    STORE_SYMBOL(vkBeginCommandBuffer)
    STORE_SYMBOL(vkCmdEndRenderPass)
    STORE_SYMBOL(vkCmdPipelineBarrier)
    STORE_SYMBOL(vkCmdBlitImage)
    STORE_SYMBOL(vkCmdCopyImage)
    STORE_SYMBOL(vkEndCommandBuffer)
    STORE_SYMBOL(vkQueueWaitIdle)
    STORE_SYMBOL(vkFreeCommandBuffers)
    STORE_SYMBOL(vkGetImageSubresourceLayout)
    STORE_SYMBOL(vkMapMemory)
    STORE_SYMBOL(vkGetSwapchainImagesKHR)
    STORE_SYMBOL(vkCreateFence)
    STORE_SYMBOL(vkCreateSemaphore)
    STORE_SYMBOL(vkDestroyRenderPass)
    STORE_SYMBOL(vkDestroySemaphore)
    STORE_SYMBOL(vkDestroyFence)
    STORE_SYMBOL(vkFreeCommandBuffers)
    STORE_SYMBOL(vkDestroyCommandPool)
    STORE_SYMBOL(vkDestroyImageView)
    STORE_SYMBOL(vkDestroySampler)
    STORE_SYMBOL(vkDestroyFramebuffer)
    STORE_SYMBOL(vkCmdClearColorImage)
    STORE_RETURN_SYMBOL(vkCmdDraw)
    STORE_RETURN_SYMBOL(vkCmdDrawIndirect)
    STORE_RETURN_SYMBOL(vkCmdDrawIndexed)
    STORE_RETURN_SYMBOL(vkCmdDrawIndexedIndirect)

    return real_pointer;
}

VkResult myvkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LINK_NAMESPACE(vkCreateInstance, "vulkan");

    if (GlobalState::isNative())
        return orig::vkCreateInstance(pCreateInfo, pAllocator, pInstance);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    VkInstanceCreateInfo createInfo = *pCreateInfo;
    // createInfo.enabledLayerCount = 1;
    // const char* layer = "VK_LAYER_KHRONOS_validation";
    // createInfo.ppEnabledLayerNames = &layer;
    
    VkResult res = orig::vkCreateInstance(&createInfo, pAllocator, pInstance);

    if (res == VK_SUCCESS) {
        /* Store the instance */
        vk::context.instance = *pInstance;
    }

    return res;
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

    if (!orig::vkGetDeviceProcAddr) return nullptr;

    return reinterpret_cast<void(*)()>(store_orig_and_return_my_symbol(pName, reinterpret_cast<void*>(orig::vkGetDeviceProcAddr(device, pName))));
}

#define GETPROCADDR(symbol) \
orig::symbol = reinterpret_cast<decltype(orig::symbol)>(orig::vkGetDeviceProcAddr(vk::context.device, #symbol));

VkResult myvkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    if (GlobalState::isNative())
        return orig::vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Store physical device */
    vk::context.physicalDevice = physicalDevice;

    /* Get memory properties of physical device */
    orig::vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vk::context.deviceMemoryProperties);

    VkResult res = orig::vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (res == VK_SUCCESS) {
        /* Store the device */
        vk::context.device = *pDevice;

        /* We are officially using Vulkan now. */
        Global::game_info.video |= GameInfo::VULKAN;
        Global::game_info.tosend = true;
        
        /* Get the address of all functions that we will be using */
        GETPROCADDR(vkCreateImageView)
        GETPROCADDR(vkCreateSampler)
        GETPROCADDR(vkCreateFramebuffer)    
        GETPROCADDR(vkGetPhysicalDeviceMemoryProperties)
        GETPROCADDR(vkCreateImage)
        GETPROCADDR(vkGetImageMemoryRequirements)
        GETPROCADDR(vkAllocateMemory)
        GETPROCADDR(vkBindImageMemory)
        GETPROCADDR(vkCreateCommandPool)
        GETPROCADDR(vkCreateDescriptorPool)
        GETPROCADDR(vkDestroyDescriptorPool)
        GETPROCADDR(vkCreateRenderPass)
        GETPROCADDR(vkDestroySwapchainKHR)
        GETPROCADDR(vkCmdBeginRenderPass)
        GETPROCADDR(vkQueueSubmit)
        GETPROCADDR(vkUnmapMemory)
        GETPROCADDR(vkFreeMemory)
        GETPROCADDR(vkDestroyImage)
        GETPROCADDR(vkAllocateCommandBuffers)
        GETPROCADDR(vkBeginCommandBuffer)
        GETPROCADDR(vkCmdEndRenderPass)
        GETPROCADDR(vkCmdPipelineBarrier)
        GETPROCADDR(vkCmdBlitImage)
        GETPROCADDR(vkCmdCopyImage)
        GETPROCADDR(vkEndCommandBuffer)
        GETPROCADDR(vkQueueWaitIdle)
        GETPROCADDR(vkFreeCommandBuffers)
        GETPROCADDR(vkGetImageSubresourceLayout)
        GETPROCADDR(vkMapMemory)
        GETPROCADDR(vkGetSwapchainImagesKHR)
        GETPROCADDR(vkCreateFence)
        GETPROCADDR(vkCreateSemaphore)
        GETPROCADDR(vkDestroyRenderPass)
        GETPROCADDR(vkDestroySemaphore)
        GETPROCADDR(vkDestroyFence)
        GETPROCADDR(vkFreeCommandBuffers)
        GETPROCADDR(vkDestroyCommandPool)
        GETPROCADDR(vkDestroyImageView)
        GETPROCADDR(vkDestroySampler)
        GETPROCADDR(vkDestroyFramebuffer)
        GETPROCADDR(vkCmdClearColorImage)
    }
    
    return res;
}

void myvkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    if (GlobalState::isNative())
        return orig::vkDestroyDevice(device, pAllocator);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Stop the screen capture */
    ScreenCapture::fini();

    return orig::vkDestroyDevice(device, pAllocator);
}

void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    if (GlobalState::isNative())
        return orig::vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Store the queue family */
    vk::context.queueFamily = queueFamilyIndex;

    return orig::vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    if (GlobalState::isNative())
        return orig::vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    VkResult res = orig::vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    if (res == VK_SUCCESS) {
        /* Store the descriptor pool */
        vk::context.descriptorPool = *pDescriptorPool;
    }
    
    return res;
}

static void destroySwapchain()
{
    ScreenCapture::fini();
    RenderHUD_Vulkan::fini();

    for (uint32_t i = 0; i < vk::context.imageCount; i++) {        
        Vulkan_Frame* fd = &vk::context.frames[i];
        Vulkan_FrameSemaphores* fsd = &vk::context.frameSemaphores[i];

        orig::vkDestroySemaphore(vk::context.device, fsd->imageAcquiredSemaphore, vk::context.allocator);
        orig::vkDestroySemaphore(vk::context.device, fsd->screenCompleteSemaphore, vk::context.allocator);
        orig::vkDestroySemaphore(vk::context.device, fsd->clearCompleteSemaphore, vk::context.allocator);
        orig::vkDestroySemaphore(vk::context.device, fsd->osdCompleteSemaphore, vk::context.allocator);
        fsd->imageAcquiredSemaphore = fsd->screenCompleteSemaphore = fsd->clearCompleteSemaphore = fsd->osdCompleteSemaphore = VK_NULL_HANDLE;

        orig::vkDestroyFence(vk::context.device, fd->fence, vk::context.allocator);
        orig::vkFreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->screenCommandBuffer);
        orig::vkFreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->clearCommandBuffer);
        orig::vkFreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->osdCommandBuffer);
        orig::vkDestroyCommandPool(vk::context.device, fd->commandPool, vk::context.allocator);
        fd->fence = VK_NULL_HANDLE;
        fd->screenCommandBuffer = VK_NULL_HANDLE;
        fd->clearCommandBuffer = VK_NULL_HANDLE;
        fd->osdCommandBuffer = VK_NULL_HANDLE;
        fd->commandPool = VK_NULL_HANDLE;

        orig::vkDestroyFramebuffer(vk::context.device, fd->framebuffer, vk::context.allocator);
        orig::vkDestroyImageView(vk::context.device, fd->backbufferView, vk::context.allocator);
        fd->framebuffer = VK_NULL_HANDLE;
        fd->backbufferView = VK_NULL_HANDLE;
    }
    
    orig::vkDestroyRenderPass(vk::context.device, vk::context.renderPass, vk::context.allocator);
}

VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    if (GlobalState::isNative())
        return orig::vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* We must add support for transfering the swapchain image from/to another image,
     * for encoding and hud. */
    VkSwapchainCreateInfoKHR newCreateInfo = *pCreateInfo;
    newCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    /* Save the color format */
    vk::context.colorFormat = pCreateInfo->imageFormat;

    /* Destroy old swapchain elements */
    if (vk::context.swapchain) {
        destroySwapchain();
    }

    VkResult res = orig::vkCreateSwapchainKHR(device, &newCreateInfo, pAllocator, pSwapchain);    
    
    vk::context.swapchain = *pSwapchain;
    vk::context.swapchainRebuild = false;

    /* Store the swapchain size */
    vk::context.width = newCreateInfo.imageExtent.width;
    vk::context.height = newCreateInfo.imageExtent.height;
    
    /* Get the currently acquired swapchain image */
    orig::vkGetSwapchainImagesKHR(device, *pSwapchain, &vk::context.imageCount, nullptr);
    
    std::vector<VkImage> swapchainImgs;
    swapchainImgs.resize(vk::context.imageCount);
    orig::vkGetSwapchainImagesKHR(device, *pSwapchain, &vk::context.imageCount, swapchainImgs.data());
    
    vk::context.frames.resize(vk::context.imageCount);
    vk::context.frameSemaphores.resize(vk::context.imageCount);
    
    VkResult err;

    /* Create render pass */
    {
        VkAttachmentDescription attachment = {};
        attachment.format = vk::context.colorFormat;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference color_attachment = {};
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        err = orig::vkCreateRenderPass(vk::context.device, &info, vk::context.allocator, &vk::context.renderPass);
        VKCHECKERROR(err);
    }
    
    /* Create command pools, command buffers, fences and semaphores */
    for (uint32_t i = 0; i < vk::context.imageCount; i++) {
        Vulkan_Frame* fd = &vk::context.frames[i];
        Vulkan_FrameSemaphores* fsd = &vk::context.frameSemaphores[i];
            
        fd->backbuffer = swapchainImgs[i];
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = vk::context.queueFamily;
            err = orig::vkCreateCommandPool(vk::context.device, &info, vk::context.allocator, &fd->commandPool);
            VKCHECKERROR(err);            
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = orig::vkAllocateCommandBuffers(vk::context.device, &info, &fd->screenCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = orig::vkAllocateCommandBuffers(vk::context.device, &info, &fd->clearCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = orig::vkAllocateCommandBuffers(vk::context.device, &info, &fd->osdCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            err = orig::vkCreateFence(vk::context.device, &info, vk::context.allocator, &fd->fence);
            VKCHECKERROR(err);
        }
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            err = orig::vkCreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->imageAcquiredSemaphore);
            VKCHECKERROR(err);
            err = orig::vkCreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->screenCompleteSemaphore);
            VKCHECKERROR(err);
            err = orig::vkCreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->clearCompleteSemaphore);
            VKCHECKERROR(err);
            err = orig::vkCreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->osdCompleteSemaphore);
            VKCHECKERROR(err);
        }
        
        // Create The Image Views
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = vk::context.colorFormat;
            info.components.r = VK_COMPONENT_SWIZZLE_R;
            info.components.g = VK_COMPONENT_SWIZZLE_G;
            info.components.b = VK_COMPONENT_SWIZZLE_B;
            info.components.a = VK_COMPONENT_SWIZZLE_A;
            VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            info.subresourceRange = image_range;
            info.image = fd->backbuffer;
            err = orig::vkCreateImageView(vk::context.device, &info, vk::context.allocator, &fd->backbufferView);
            VKCHECKERROR(err);
        }
        
        /* Create Framebuffer */
        {
            VkImageView attachment[1];
            VkFramebufferCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = vk::context.renderPass;
            info.attachmentCount = 1;
            info.pAttachments = attachment;
            info.width = vk::context.width;
            info.height = vk::context.height;
            info.layers = 1;
            attachment[0] = fd->backbufferView;
            VkResult err = orig::vkCreateFramebuffer(device, &info, vk::context.allocator, &fd->framebuffer);
            VKCHECKERROR(err);
        }
    }
    
    return res;
}

void vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    if (GlobalState::isNative())
        return orig::vkDestroySwapchainKHR(device, swapchain, pAllocator);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    if (vk::context.swapchain == swapchain) {
        destroySwapchain();
        vk::context.swapchain = VK_NULL_HANDLE;
    }

    return orig::vkDestroySwapchainKHR(device, swapchain, pAllocator);
}

VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    if (GlobalState::isNative())
        return orig::vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    if (Global::skipping_draw) {
        /* We don't acquire an image, so that we don't need to call vkQueuePresentKHR()
         * later. However, this function also signal either a semaphore or a
         * fence, so we still need to signal either one. There does not seem to
         * be specific functions for that, so I'm using an empty command submitted
         * to a queue. */
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 0;
        info.waitSemaphoreCount = 0;
        if (semaphore) {
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &semaphore;
        }
        else {
            info.signalSemaphoreCount = 0;
        }
        orig::vkQueueSubmit(vk::context.graphicsQueue, 1, &info, fence);
        return VK_SUCCESS;
    }

    VkResult res = orig::vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    
    /* Store image index */
    vk::context.frameIndex = *pImageIndex;
    return res;
}

VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    if (GlobalState::isNative())
        return orig::vkQueuePresentKHR(queue, pPresentInfo);

    DEBUGLOGCALL(LCF_WINDOW | LCF_VULKAN);

    /* Store the graphic queue */
    vk::context.graphicsQueue = queue;

    /* We check that only one swapchain is being presented.
     * Otherwise, I don't know what to do. */
    if (pPresentInfo->swapchainCount != 1) {
        debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_WARNING, "Multiple swapchains are being presented");
    }

    if (pPresentInfo->waitSemaphoreCount == 0) {
        vk::context.currentSemaphore = VK_NULL_HANDLE;
    }
    else if (pPresentInfo->waitSemaphoreCount == 1) {
        vk::context.currentSemaphore = pPresentInfo->pWaitSemaphores[0];
    }
    else {
        debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "   Waiting on multiple semaphores");            
        vk::context.currentSemaphore = VK_NULL_HANDLE;
    }

    if (Global::skipping_draw) {        
        /* If skipping draw, we must still wait on all semaphores, so we push an
        * empty command */
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 0;
        info.waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
        info.pWaitSemaphores = pPresentInfo->pWaitSemaphores;
        info.signalSemaphoreCount = 0;
        orig::vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
    }

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_Vulkan renderHUD;
    
    VkResult ret;
    frameBoundary([&] () {
        if (vk::context.swapchainRebuild)
            return;
        
        VkPresentInfoKHR pi = *pPresentInfo;

        /* Wait on the last semaphore that was used by our code */
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &vk::context.currentSemaphore;

        /* Present the queue with the stored image index, because we will 
         * acquire other images when presenting again. */
        pi.pImageIndices = &vk::context.frameIndex;
        // debuglogstdio(LCF_WINDOW | LCF_VULKAN, "    vkQueuePresentKHR wait on semaphore %llx", vk::context.currentSemaphore);
        ret = orig::vkQueuePresentKHR(queue, &pi);
        
        if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR) {
            vk::context.swapchainRebuild = true;
            return;
        }
        
        /* TODO: Use fence to delay waiting on queue */
        orig::vkQueueWaitIdle(vk::context.graphicsQueue);

    }, renderHUD);

    return ret;
}

}
