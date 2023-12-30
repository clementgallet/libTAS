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

#include "RenderHUD_Vulkan.h"

#include "logging.h"
#include "hook.h"
#include "ScreenCapture.h"
#include "GlobalState.h"
#include "rendering/vulkanwrappers.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_vulkan.h"

namespace libtas {

DECLARE_ORIG_POINTER(vkGetInstanceProcAddr)
DECLARE_ORIG_POINTER(vkBeginCommandBuffer)
DECLARE_ORIG_POINTER(vkCmdBeginRenderPass)
DECLARE_ORIG_POINTER(vkCmdEndRenderPass)
DECLARE_ORIG_POINTER(vkEndCommandBuffer)
DECLARE_ORIG_POINTER(vkQueueSubmit)
DECLARE_ORIG_POINTER(vkQueueWaitIdle)

#define VKCHECKERROR(err) \
do { if (err < 0) debuglogstdio(LCF_WINDOW | LCF_VULKAN | LCF_ERROR, "Vulkan error: %d", err); } while (0)

RenderHUD_Vulkan::~RenderHUD_Vulkan() {
    fini();
}

void RenderHUD_Vulkan::init() {
    
    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return orig::vkGetInstanceProcAddr(vk::context.instance, function_name); });
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk::context.instance;
    init_info.PhysicalDevice = vk::context.physicalDevice;
    init_info.Device = vk::context.device;
    init_info.QueueFamily = vk::context.queueFamily;
    init_info.Queue = vk::context.graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = vk::context.descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = vk::context.imageCount;
    init_info.ImageCount = vk::context.imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = vk::context.allocator;
    init_info.CheckVkResultFn = vk::checkVkResult;
    ImGui_ImplVulkan_Init(&init_info, vk::context.renderPass);
}

void RenderHUD_Vulkan::fini() {
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}

void RenderHUD_Vulkan::newFrame()
{
    if (!ImGui::GetCurrentContext()) {
        ImGui::CreateContext();
        init();
    }
    ImGui_ImplVulkan_NewFrame();

    RenderHUD::newFrame();
    
    /* Update semaphore index */
    vk::context.semaphoreIndex = (vk::context.semaphoreIndex + 1) % vk::context.imageCount;
}

void RenderHUD_Vulkan::render()
{
    if (!ImGui::GetCurrentContext())
        return;
        
    ImGui::Render();

    VkResult err;
    Vulkan_Frame* fd = &vk::context.frames[vk::context.frameIndex];
    
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = orig::vkBeginCommandBuffer(fd->commandBuffer, &info);
        VKCHECKERROR(err);
    }
    
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = vk::context.renderPass;
        info.framebuffer = fd->framebuffer;
        info.renderArea.extent.width = vk::context.width;
        info.renderArea.extent.height = vk::context.height;
        info.clearValueCount = 1;
        info.pClearValues = &vk::context.clearValue;
        orig::vkCmdBeginRenderPass(fd->commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }
    
    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->commandBuffer);
    
    // Submit command buffer
    orig::vkCmdEndRenderPass(fd->commandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->commandBuffer;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &vk::context.currentSemaphore;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &vk::context.frameSemaphores[vk::context.semaphoreIndex].osdCompleteSemaphore;
    
        err = orig::vkEndCommandBuffer(fd->commandBuffer);
        VKCHECKERROR(err);

        debuglogstdio(LCF_VULKAN, "    vkQueueSubmit wait on %llx and signal %llx and semindex %d", info.pWaitSemaphores[0], info.pSignalSemaphores[0], vk::context.semaphoreIndex);

        err = orig::vkQueueSubmit(vk::context.graphicsQueue, 1, &info, VK_NULL_HANDLE);
        VKCHECKERROR(err);

        vk::context.currentSemaphore = vk::context.frameSemaphores[vk::context.semaphoreIndex].osdCompleteSemaphore;
        
        /* TODO: Get rid of this. For now it is necessary */
        orig::vkQueueWaitIdle(vk::context.graphicsQueue);
    }
}

}
