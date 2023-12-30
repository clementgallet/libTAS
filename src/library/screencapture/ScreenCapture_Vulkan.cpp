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

#include "ScreenCapture_Vulkan.h"
#include "hook.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"
#include "rendering/vulkanwrappers.h"

namespace libtas {

DECLARE_ORIG_POINTER(vkCreateImage)
DECLARE_ORIG_POINTER(vkGetImageMemoryRequirements)
DECLARE_ORIG_POINTER(vkAllocateMemory)
DECLARE_ORIG_POINTER(vkBindImageMemory)
DECLARE_ORIG_POINTER(vkUnmapMemory)
DECLARE_ORIG_POINTER(vkFreeMemory)
DECLARE_ORIG_POINTER(vkDestroyImage)
DECLARE_ORIG_POINTER(vkAllocateCommandBuffers)
DECLARE_ORIG_POINTER(vkBeginCommandBuffer)
DECLARE_ORIG_POINTER(vkCmdPipelineBarrier)
DECLARE_ORIG_POINTER(vkCmdBlitImage)
DECLARE_ORIG_POINTER(vkCmdCopyImage)
DECLARE_ORIG_POINTER(vkEndCommandBuffer)
DECLARE_ORIG_POINTER(vkQueueSubmit)
DECLARE_ORIG_POINTER(vkFreeCommandBuffers)
DECLARE_ORIG_POINTER(vkGetImageSubresourceLayout)
DECLARE_ORIG_POINTER(vkMapMemory)
DECLARE_ORIG_POINTER(vkAcquireNextImageKHR)

int ScreenCapture_Vulkan::init()
{
    if (ScreenCapture_Impl::init() < 0)
        return -1;
    
    pixelSize = 4;
    
    return ScreenCapture_Impl::postInit();
}

void ScreenCapture_Vulkan::initScreenSurface()
{
    VkResult res;
    
    /* Create the image info */
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    /* Create the image */
    if ((res = orig::vkCreateImage(vk::context.device, &imageInfo, nullptr, &vkScreenImage)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkCreateImage failed with error %d", res);
    }
    
    /* Create memory to back up the image */
    VkMemoryRequirements memRequirements;
    orig::vkGetImageMemoryRequirements(vk::context.device, vkScreenImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    allocInfo.memoryTypeIndex = vk::getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if ((res = orig::vkAllocateMemory(vk::context.device, &allocInfo, nullptr, &vkScreenImageMemory)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkAllocateMemory failed with error %d", res);
    }

    orig::vkBindImageMemory(vk::context.device, vkScreenImage, vkScreenImageMemory, 0);
}

void ScreenCapture_Vulkan::destroyScreenSurface()
{
    /* Delete the Vulkan image */
    if (vkScreenImageMemory != VK_NULL_HANDLE) {
        orig::vkFreeMemory(vk::context.device, vkScreenImageMemory, nullptr);
        vkScreenImageMemory = VK_NULL_HANDLE;
    }
    if (vkScreenImage != VK_NULL_HANDLE) {
        orig::vkDestroyImage(vk::context.device, vkScreenImage, nullptr);
        vkScreenImage = VK_NULL_HANDLE;
    }
}

const char* ScreenCapture_Vulkan::getPixelFormat()
{
    switch(vk::context.colorFormat) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
            return "BGRA";
        default:
            debuglogstdio(LCF_DUMP | LCF_VULKAN | LCF_ERROR, "  Unsupported pixel format %d", vk::context.colorFormat);
            return "RGBA";
    }
}

int ScreenCapture_Vulkan::copyScreenToSurface()
{
    GlobalNative gn;

    VkResult res;
    
    VkCommandBuffer cmdBuffer = vk::context.frames[vk::context.frameIndex].screenCommandBuffer;
    VkImage backbuffer = vk::context.frames[vk::context.frameIndex].backbuffer;

    VkCommandBufferBeginInfo cmdBufInfo{};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if ((res = orig::vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkBeginCommandBuffer failed with error %d", res);
    }
    
    /* Transition destination image to transfer destination layout */
    VkImageMemoryBarrier dstBarrier{};
    dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    dstBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    dstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.image = vkScreenImage;
    dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstBarrier.subresourceRange.baseMipLevel = 0;
    dstBarrier.subresourceRange.levelCount = 1;
    dstBarrier.subresourceRange.baseArrayLayer = 0;
    dstBarrier.subresourceRange.layerCount = 1;
    dstBarrier.srcAccessMask = 0;
    dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &dstBarrier
    );

    VkImageMemoryBarrier srcBarrier{};
    srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    srcBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    srcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.image = backbuffer;
    srcBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcBarrier.subresourceRange.baseMipLevel = 0;
    srcBarrier.subresourceRange.levelCount = 1;
    srcBarrier.subresourceRange.baseArrayLayer = 0;
    srcBarrier.subresourceRange.layerCount = 1;
    srcBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &srcBarrier
    );

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width = width;
	imageCopyRegion.extent.height = height;
	imageCopyRegion.extent.depth = 1;

	/* Issue the copy command */
	orig::vkCmdCopyImage(
		cmdBuffer,
		backbuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vkScreenImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	/* Transition destination image to general layout, which is the required layout for mapping the image memory later on */
    dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    dstBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &dstBarrier
    );

    srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    srcBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    srcBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    srcBarrier.image = backbuffer;
    srcBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &srcBarrier
    );

    /* Flush the command buffer */
    if ((res = orig::vkEndCommandBuffer(cmdBuffer)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    if (!vk::context.currentSemaphore) {
        debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "    No semaphore to wait on");
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = VK_NULL_HANDLE;            
    }
    else {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &vk::context.currentSemaphore;
    }
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk::context.frameSemaphores[vk::context.semaphoreIndex].screenCompleteSemaphore;

    // debuglogstdio(LCF_VULKAN, "    vkQueueSubmit wait on %llx and signal %llx and semindex %d", submitInfo.pWaitSemaphores[0], submitInfo.pSignalSemaphores[0], vk::context.semaphoreIndex);

    if ((res = orig::vkQueueSubmit(vk::context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
    }

    vk::context.currentSemaphore = submitInfo.pSignalSemaphores[0];
    
    return size;
}

int ScreenCapture_Vulkan::getPixelsFromSurface(uint8_t **pixels, bool draw)
{
    if (pixels) {
        *pixels = winpixels.data();
    }

    if (!draw)
        return size;

    GlobalNative gn;

    VkResult res;

    /* Get layout of the image (including row pitch) */
	VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	orig::vkGetImageSubresourceLayout(vk::context.device, vkScreenImage, &subResource, &subResourceLayout);

	/* Map image memory so we can start copying from it */
	const char* data;
    if ((res = orig::vkMapMemory(vk::context.device, vkScreenImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
    }
    
    /* Copy image pixels respecting the image layout. */
    data += subResourceLayout.offset;
    VkDeviceSize s = 0;
    int h = 0;
    while ((s < subResourceLayout.size) && (h < height)) {
        memcpy(&winpixels[h*width*pixelSize], data, width*pixelSize);
        data += subResourceLayout.rowPitch;
        s += subResourceLayout.rowPitch;
        h++;
    }

    if (h != height)
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "Mismatch between Vulkan internal image height (%d) and registered height (%d)", h, height);
    
    orig::vkUnmapMemory(vk::context.device, vkScreenImageMemory);
    
    return size;
}

int ScreenCapture_Vulkan::copySurfaceToScreen()
{
    GlobalNative gn;

    /* Acquire an image from the swapchain */
    VkResult res = orig::vkAcquireNextImageKHR(vk::context.device, vk::context.swapchain, UINT64_MAX, vk::context.frameSemaphores[vk::context.semaphoreIndex].imageAcquiredSemaphore, VK_NULL_HANDLE, &vk::context.frameIndex);
    if ((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR))
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkAcquireNextImageKHR failed with error %d", res);

    // debuglogstdio(LCF_VULKAN, "vkAcquireNextImageKHR signals %llx and returns image index %d", vk::context.frameSemaphores[vk::context.semaphoreIndex].imageAcquiredSemaphore, vk::context.frameIndex);
    
    VkCommandBuffer cmdBuffer = vk::context.frames[vk::context.frameIndex].screenCommandBuffer;
    VkImage backbuffer = vk::context.frames[vk::context.frameIndex].backbuffer;

    VkCommandBufferBeginInfo cmdBufInfo{};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if ((res = orig::vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkBeginCommandBuffer failed with error %d", res);
    }

    /* Transition destination image to transfer destination layout */
    VkImageMemoryBarrier dstBarrier{};
    dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    dstBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    dstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.image = backbuffer;
    dstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstBarrier.subresourceRange.baseMipLevel = 0;
    dstBarrier.subresourceRange.levelCount = 1;
    dstBarrier.subresourceRange.baseArrayLayer = 0;
    dstBarrier.subresourceRange.layerCount = 1;
    dstBarrier.srcAccessMask = 0;
    dstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &dstBarrier
    );

    VkImageMemoryBarrier srcBarrier{};
    srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    srcBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    srcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.image = vkScreenImage;
    srcBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcBarrier.subresourceRange.baseMipLevel = 0;
    srcBarrier.subresourceRange.levelCount = 1;
    srcBarrier.subresourceRange.baseArrayLayer = 0;
    srcBarrier.subresourceRange.layerCount = 1;
    srcBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    srcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &srcBarrier
    );

    /* Otherwise use image copy (requires us to manually flip components) */
    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = width;
    imageCopyRegion.extent.height = height;
    imageCopyRegion.extent.depth = 1;

    /* Issue the copy command */
    orig::vkCmdCopyImage(
        cmdBuffer,
        vkScreenImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        backbuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopyRegion);

    /* Transition destination image to general layout, which is the required layout for mapping the image memory later on */
    dstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    dstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    dstBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    dstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &dstBarrier
    );

    srcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    srcBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    srcBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    srcBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    orig::vkCmdPipelineBarrier(cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &srcBarrier
    );

    /* Flush the command buffer */
    if ((res = orig::vkEndCommandBuffer(cmdBuffer)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vk::context.frameSemaphores[vk::context.semaphoreIndex].imageAcquiredSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vk::context.frameSemaphores[vk::context.semaphoreIndex].screenCompleteSemaphore;

    // debuglogstdio(LCF_VULKAN, "    vkQueueSubmit wait on %llx and signal %llx and semindex %d", submitInfo.pWaitSemaphores[0], submitInfo.pSignalSemaphores[0], vk::context.semaphoreIndex);

    if ((res = orig::vkQueueSubmit(vk::context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS) {
        debuglogstdio(LCF_VULKAN | LCF_ERROR, "vkEndCommandBuffer failed with error %d", res);
    }

    vk::context.currentSemaphore = vk::context.frameSemaphores[vk::context.semaphoreIndex].screenCompleteSemaphore;

    return 0;
}

}
