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

#include "vulkanwrappers.h"

#include "vulkanloader.h"
#include "hook.h"
#include "logging.h"
#include "renderhud/RenderHUD_Vulkan.h"
#include "screencapture/ScreenCapture.h"
#include "frame.h"
#include "global.h"
#include "GlobalState.h"

#include <string.h>
#include <dlfcn.h>

#define RETURN_SYMBOL_DETAILED(FUNC, NEW_FUNC) \
    if (! NEW_FUNC) { \
        LOG(LL_WARN, LCF_HOOK | LCF_VULKAN,"  Trying to hook Vulkan function %s that we didn't define", symbol); \
    } \
    if (!strcmp(symbol, "vk" #FUNC)) { \
        if (real_pointer == reinterpret_cast<void*>(NEW_FUNC)) { \
            LOG(LL_WARN, LCF_HOOK | LCF_VULKAN,"  Vulkan proc function failed at finding the real function %s, using the standard symbol loading method instead", symbol); \
            GlobalNative gn; \
            void* handle = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_DEEPBIND); \
            if (handle != NULL) { \
                vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(dlsym(handle, symbol)); \
                dlclose(handle); \
            } \
        } \
        else { \
            vkProcs.FUNC = reinterpret_cast<decltype(&vk##FUNC)>(real_pointer); \
        } \
        LOG(LL_DEBUG, LCF_VULKAN,"  return my symbol %p, real function in %p", reinterpret_cast<void*>(NEW_FUNC), vkProcs.FUNC); \
        return reinterpret_cast<void*>(NEW_FUNC); \
    }

#define STORE_RETURN_SYMBOL(FUNC) RETURN_SYMBOL_DETAILED(FUNC, libtas::vk##FUNC)
#define STORE_RETURN_SYMBOL_CUSTOM(FUNC) RETURN_SYMBOL_DETAILED(FUNC, myvk##FUNC)

namespace libtas {

Vulkan_Context vk::context = {};

#define VKCHECKERROR(err) \
do { if (err < 0) LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "Vulkan error: %d", err); } while (0)

void vk::checkVkResult(VkResult err)
{
    if (err == 0)
        return;
    LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "Vulkan error: %d", err);
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

    LOG(LL_DEBUG, LCF_WINDOW | LCF_VULKAN, "Could not find a suitable memory type");
    return 0;
}

#define VKFUNCSKIPDRAW(NAME, DECL, ARGS) \
void vk##NAME DECL\
{\
    LINK_VK_POINTER(NAME);\
    LOGTRACE(LCF_VULKAN);\
    if (!Global::skipping_draw)\
        return vkProcs.NAME ARGS;\
}

VKFUNCSKIPDRAW(CmdDraw, (VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance), (commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance))
VKFUNCSKIPDRAW(CmdDrawIndirect, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride), (commandBuffer, buffer, offset, drawCount, stride))
// VKFUNCSKIPDRAW(CmdDrawIndirectCount, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride), (commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride))
VKFUNCSKIPDRAW(CmdDrawIndexed, (VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance), (commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance))
VKFUNCSKIPDRAW(CmdDrawIndexedIndirect, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride), (commandBuffer, buffer, offset, drawCount, stride))
// VKFUNCSKIPDRAW(CmdDrawIndexedIndirectCount, (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride), (commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride))

/* If the game uses the vkGetInstanceProcAddr functions to access to a function
 * that we hook, we must return our function and store the original pointers
 * so that we can call the real function.
 */
static void* return_my_symbol(const char* symbol, void* real_pointer) {

    if (!real_pointer || !symbol)
        return real_pointer;

    STORE_RETURN_SYMBOL(CreateInstance)
    STORE_RETURN_SYMBOL(CreateSwapchainKHR)
    STORE_RETURN_SYMBOL(AcquireNextImageKHR)
    STORE_RETURN_SYMBOL(QueuePresentKHR)
    STORE_RETURN_SYMBOL(CreateDevice)
    STORE_RETURN_SYMBOL(DestroyDevice)
    STORE_RETURN_SYMBOL(GetDeviceProcAddr)
    STORE_RETURN_SYMBOL(GetDeviceQueue)
    STORE_RETURN_SYMBOL(DestroySwapchainKHR)
    STORE_RETURN_SYMBOL(CmdDraw)
    STORE_RETURN_SYMBOL(CmdDrawIndirect)
    // STORE_RETURN_SYMBOL(CmdDrawIndirectCount)
    STORE_RETURN_SYMBOL(CmdDrawIndexed)
    STORE_RETURN_SYMBOL(CmdDrawIndexedIndirect)
    // STORE_RETURN_SYMBOL(CmdDrawIndexedIndirectCount)

    return real_pointer;
}

VkResult vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    LINK_VK_POINTER(CreateInstance);

    if (GlobalState::isNative())
        return vkProcs.CreateInstance(pCreateInfo, pAllocator, pInstance);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    VkInstanceCreateInfo createInfo = *pCreateInfo;
    // createInfo.enabledLayerCount = 1;
    // const char* layer = "VK_LAYER_KHRONOS_validation";
    // createInfo.ppEnabledLayerNames = &layer;
    
    VkResult res = vkProcs.CreateInstance(&createInfo, pAllocator, pInstance);

    if (res == VK_SUCCESS) {
        /* Store the instance */
        vk::context.instance = *pInstance;
        vk::context.allocator = pAllocator;
    }

    return res;
}

PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    LOG(LL_TRACE, LCF_HOOK | LCF_VULKAN, "%s call with symbol %s", __func__, pName);
    LINK_VK_POINTER(GetInstanceProcAddr);

    if (!vkProcs.GetInstanceProcAddr) return nullptr;

    return reinterpret_cast<PFN_vkVoidFunction>(return_my_symbol(pName, reinterpret_cast<void*>(vkProcs.GetInstanceProcAddr(instance, pName))));
}

PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char* pName)
{
    LOG(LL_TRACE, LCF_HOOK | LCF_VULKAN, "%s call with symbol %s", __func__, pName);
    LINK_VK_POINTER(GetDeviceProcAddr);

    if (!vkProcs.GetDeviceProcAddr) return nullptr;

    return reinterpret_cast<PFN_vkVoidFunction>(return_my_symbol(pName, reinterpret_cast<void*>(vkProcs.GetDeviceProcAddr(device, pName))));
}

/* Function to pass to our function pointer loader */
PFN_vkVoidFunction vkGetProcAddr(const char *proc)
{
    PFN_vkVoidFunction func = nullptr;

    if (vkProcs.GetDeviceProcAddr) {
        func = vkProcs.GetDeviceProcAddr(vk::context.device, proc);
    }
    if (!func && vkProcs.GetInstanceProcAddr) {
        func = vkProcs.GetInstanceProcAddr(vk::context.instance, proc);
    }

    if (!func) {
        GlobalNative gn;
        void* handle = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_DEEPBIND);
        if (handle != NULL) {
            func = reinterpret_cast<PFN_vkVoidFunction>(dlsym(handle, proc));
            dlclose(handle);
        }
    }
    return func;
}

VkResult vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    LINK_VK_POINTER(CreateDevice);

    if (GlobalState::isNative())
        return vkProcs.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    /* Store physical device */
    vk::context.physicalDevice = physicalDevice;

    VkResult res;
    /* The device creation must be performed natively, so that it can import
     * the correct vkGetDeviceProcAddr function with dlsym(), without being
     * messed up by libtas.
     */
    NATIVECALL(res = vkProcs.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice));

    if (res == VK_SUCCESS) {
        /* Store the device */
        vk::context.device = *pDevice;

        /* We are officially using Vulkan now. */
        Global::game_info.video |= GameInfo::VULKAN;
        Global::game_info.tosend = true;
        
        /* Get the address of all functions that we will be using */
        vk_load_procs(vkGetProcAddr);

        /* Get memory properties of physical device */
        vkProcs.GetPhysicalDeviceMemoryProperties(physicalDevice, &vk::context.deviceMemoryProperties);

        /* Create the descriptor pool that will create descriptor sets for the
         * font texture and game window texture */
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 2;
            pool_info.poolSizeCount = 1;
            pool_info.pPoolSizes = pool_sizes;
            VkResult err = vkProcs.CreateDescriptorPool(vk::context.device, &pool_info, vk::context.allocator, &vk::context.descriptorPool);
            VKCHECKERROR(err);
        }
    }
    
    return res;
}

void vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    if (GlobalState::isNative())
        return vkProcs.DestroyDevice(device, pAllocator);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    if (vk::context.descriptorPool) {
        vkProcs.DestroyDescriptorPool(device, vk::context.descriptorPool, pAllocator);
        vk::context.descriptorPool = VK_NULL_HANDLE;
    }

    /* Stop the screen capture */
    ScreenCapture::fini();

    return vkProcs.DestroyDevice(device, pAllocator);
}

void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    if (GlobalState::isNative())
        return vkProcs.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    /* Store the queue family */
    vk::context.queueFamily = queueFamilyIndex;

    vkProcs.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

    if (pQueue) {
        vk::context.graphicsQueue = *pQueue;
    }
}

static void destroySwapchain()
{
    ScreenCapture::fini();
    RenderHUD_Vulkan::fini();

    for (uint32_t i = 0; i < vk::context.imageCount; i++) {        
        Vulkan_Frame* fd = &vk::context.frames[i];
        Vulkan_FrameSemaphores* fsd = &vk::context.frameSemaphores[i];

        vkProcs.DestroySemaphore(vk::context.device, fsd->imageAcquiredSemaphore, vk::context.allocator);
        vkProcs.DestroySemaphore(vk::context.device, fsd->screenCompleteSemaphore, vk::context.allocator);
        vkProcs.DestroySemaphore(vk::context.device, fsd->clearCompleteSemaphore, vk::context.allocator);
        vkProcs.DestroySemaphore(vk::context.device, fsd->osdCompleteSemaphore, vk::context.allocator);
        fsd->imageAcquiredSemaphore = fsd->screenCompleteSemaphore = fsd->clearCompleteSemaphore = fsd->osdCompleteSemaphore = VK_NULL_HANDLE;

        vkProcs.DestroyFence(vk::context.device, fd->fence, vk::context.allocator);
        vkProcs.FreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->screenCommandBuffer);
        vkProcs.FreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->clearCommandBuffer);
        vkProcs.FreeCommandBuffers(vk::context.device, fd->commandPool, 1, &fd->osdCommandBuffer);
        vkProcs.DestroyCommandPool(vk::context.device, fd->commandPool, vk::context.allocator);
        fd->fence = VK_NULL_HANDLE;
        fd->screenCommandBuffer = VK_NULL_HANDLE;
        fd->clearCommandBuffer = VK_NULL_HANDLE;
        fd->osdCommandBuffer = VK_NULL_HANDLE;
        fd->commandPool = VK_NULL_HANDLE;

        vkProcs.DestroyFramebuffer(vk::context.device, fd->framebuffer, vk::context.allocator);
        vkProcs.DestroyImageView(vk::context.device, fd->backbufferView, vk::context.allocator);
        fd->framebuffer = VK_NULL_HANDLE;
        fd->backbufferView = VK_NULL_HANDLE;
    }
    
    vkProcs.DestroyRenderPass(vk::context.device, vk::context.renderPass, vk::context.allocator);
    vk::context.renderPass = VK_NULL_HANDLE;
    vk::context.imageCount = 0;
    vk::context.frameIndex = 0;
    vk::context.currentSemaphore = VK_NULL_HANDLE;
    vk::context.frames.clear();
    vk::context.frameSemaphores.clear();
}

VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    if (GlobalState::isNative())
        return vkProcs.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    /* We must add support for transfering the swapchain image from/to another image,
     * for encoding and hud. */
    VkSwapchainCreateInfoKHR newCreateInfo = *pCreateInfo;
    newCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    /* Save the color format */
    vk::context.colorFormat = pCreateInfo->imageFormat;

    VkResult res = vkProcs.CreateSwapchainKHR(device, &newCreateInfo, pAllocator, pSwapchain);
    if (res != VK_SUCCESS) {
        VKCHECKERROR(res);
        return res;
    }
    
    uint32_t imageCount = 0;
    VkResult err = vkProcs.GetSwapchainImagesKHR(device, *pSwapchain, &imageCount, nullptr);
    if (err != VK_SUCCESS || imageCount == 0) {
        if (err == VK_SUCCESS) {
            err = VK_ERROR_INITIALIZATION_FAILED;
        }
        LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "Failed to query swapchain image count: %d", err);
        vkProcs.DestroySwapchainKHR(device, *pSwapchain, pAllocator);
        *pSwapchain = VK_NULL_HANDLE;
        return err;
    }

    std::vector<VkImage> swapchainImgs;
    swapchainImgs.resize(imageCount);
    err = vkProcs.GetSwapchainImagesKHR(device, *pSwapchain, &imageCount, swapchainImgs.data());
    if (err != VK_SUCCESS) {
        LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "Failed to query swapchain images: %d", err);
        vkProcs.DestroySwapchainKHR(device, *pSwapchain, pAllocator);
        *pSwapchain = VK_NULL_HANDLE;
        return err;
    }

    /* Destroy old swapchain elements */
    if (vk::context.swapchain) {
        destroySwapchain();
    }

    vk::context.swapchain = *pSwapchain;
    vk::context.swapchainRebuild = false;
    vk::context.frameIndex = 0;
    vk::context.currentSemaphore = VK_NULL_HANDLE;

    /* Store the swapchain size */
    vk::context.width = newCreateInfo.imageExtent.width;
    vk::context.height = newCreateInfo.imageExtent.height;
    vk::context.imageCount = imageCount;
    
    vk::context.frames.resize(vk::context.imageCount);
    vk::context.frameSemaphores.resize(vk::context.imageCount);

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
        err = vkProcs.CreateRenderPass(vk::context.device, &info, vk::context.allocator, &vk::context.renderPass);
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
            err = vkProcs.CreateCommandPool(vk::context.device, &info, vk::context.allocator, &fd->commandPool);
            VKCHECKERROR(err);            
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = vkProcs.AllocateCommandBuffers(vk::context.device, &info, &fd->screenCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = vkProcs.AllocateCommandBuffers(vk::context.device, &info, &fd->clearCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = fd->commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            err = vkProcs.AllocateCommandBuffers(vk::context.device, &info, &fd->osdCommandBuffer);
            VKCHECKERROR(err);
        }
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            err = vkProcs.CreateFence(vk::context.device, &info, vk::context.allocator, &fd->fence);
            VKCHECKERROR(err);
        }
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            err = vkProcs.CreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->imageAcquiredSemaphore);
            VKCHECKERROR(err);
            err = vkProcs.CreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->screenCompleteSemaphore);
            VKCHECKERROR(err);
            err = vkProcs.CreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->clearCompleteSemaphore);
            VKCHECKERROR(err);
            err = vkProcs.CreateSemaphore(vk::context.device, &info, vk::context.allocator, &fsd->osdCompleteSemaphore);
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
            err = vkProcs.CreateImageView(vk::context.device, &info, vk::context.allocator, &fd->backbufferView);
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
            VkResult err = vkProcs.CreateFramebuffer(device, &info, vk::context.allocator, &fd->framebuffer);
            VKCHECKERROR(err);
        }
    }
    
    return res;
}

void vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    if (GlobalState::isNative())
        return vkProcs.DestroySwapchainKHR(device, swapchain, pAllocator);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    if (vk::context.swapchain == swapchain) {
        destroySwapchain();
        vk::context.swapchain = VK_NULL_HANDLE;
    }

    return vkProcs.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    if (GlobalState::isNative())
        return vkProcs.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    if (Global::skipping_draw) {
        /* We don't acquire an image, so that we don't need to call vkQueuePresentKHR()
         * later. However, this function also signal either a semaphore or a
         * fence, so we still need to signal either one. There does not seem to
         * be specific functions for that, so I'm using an empty command submitted
         * to a queue. */
        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 0;
        info.waitSemaphoreCount = 0;
        if (semaphore) {
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &semaphore;
            info.pWaitDstStageMask = &stageFlags;
        }
        else {
            info.signalSemaphoreCount = 0;
        }
        if (vk::context.graphicsQueue == VK_NULL_HANDLE) {
            LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "Cannot signal skipped draw without a graphics queue");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        VkResult submitRes = vkProcs.QueueSubmit(vk::context.graphicsQueue, 1, &info, fence);
        if (submitRes != VK_SUCCESS) {
            VKCHECKERROR(submitRes);
            return submitRes;
        }
        
        /* Fill a meaningful value to the returned image index */
        if (pImageIndex) {
            *pImageIndex = 0;
        }
        
        return VK_SUCCESS;
    }

    VkResult res = vkProcs.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    if (res != VK_SUCCESS) {
        return res;
    }
    
    /* Store image index */
    if (!pImageIndex) {
        LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "vkAcquireNextImageKHR returned success with a null image index pointer");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vk::context.frameIndex = *pImageIndex;
    return res;
}

VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    if (GlobalState::isNative())
        return vkProcs.QueuePresentKHR(queue, pPresentInfo);

    LOGTRACE(LCF_WINDOW | LCF_VULKAN);

    /* Store the graphic queue */
    vk::context.graphicsQueue = queue;

    /* We check that only one swapchain is being presented.
     * Otherwise, I don't know what to do. */
    if (pPresentInfo->swapchainCount != 1) {
        LOG(LL_WARN, LCF_WINDOW | LCF_VULKAN, "Multiple swapchains are being presented");
    }

    if (pPresentInfo->waitSemaphoreCount == 0) {
        vk::context.currentSemaphore = VK_NULL_HANDLE;
    }
    else if (pPresentInfo->waitSemaphoreCount == 1) {
        vk::context.currentSemaphore = pPresentInfo->pWaitSemaphores[0];
    }
    else {
        LOG(LL_ERROR, LCF_WINDOW | LCF_VULKAN, "   Waiting on multiple semaphores");            
        vk::context.currentSemaphore = VK_NULL_HANDLE;
    }

    if (Global::skipping_draw) {        
        /* If skipping draw, we must still wait on all semaphores, so we push an
        * empty command */
        VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 0;
        info.waitSemaphoreCount = pPresentInfo->waitSemaphoreCount;
        info.pWaitSemaphores = pPresentInfo->pWaitSemaphores;
        info.pWaitDstStageMask = &stageFlags;
        info.signalSemaphoreCount = 0;
        VkResult submitRes = vkProcs.QueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
        if (submitRes != VK_SUCCESS) {
            VKCHECKERROR(submitRes);
            return submitRes;
        }
    }

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_Vulkan renderHUD;
    
    VkResult ret = VK_SUCCESS;
    frameBoundary([&] () {
        if (vk::context.swapchainRebuild)
            return;
        
        VkPresentInfoKHR pi = *pPresentInfo;

        /* Wait on the last semaphore that was used by our code when there is exactly one.
         * Otherwise preserve the original semaphore array instead of passing a null handle. */
        if (vk::context.currentSemaphore != VK_NULL_HANDLE) {
            pi.waitSemaphoreCount = 1;
            pi.pWaitSemaphores = &vk::context.currentSemaphore;
        }

        /* Present the queue with the stored image index, because we will 
         * acquire other images when presenting again. */
        pi.pImageIndices = &vk::context.frameIndex;
        // LOG(LL_DEBUG, LCF_WINDOW | LCF_VULKAN, "    vkQueuePresentKHR wait on semaphore %llx", vk::context.currentSemaphore);
        ret = vkProcs.QueuePresentKHR(queue, &pi);
        
        if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR) {
            vk::context.swapchainRebuild = true;
            vk::context.currentSemaphore = VK_NULL_HANDLE;
            return;
        }
        
        /* TODO: Use fence to delay waiting on queue */
        vkProcs.QueueWaitIdle(vk::context.graphicsQueue);

    }, renderHUD);

    return ret;
}

}
