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

RenderHUD_Vulkan::~RenderHUD_Vulkan() {
    fini();
}

void RenderHUD_Vulkan::fini() {
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}

void RenderHUD_Vulkan::newFrame()
{
    if (!ImGui::GetCurrentContext()) {
        ImGui::CreateContext();
        
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = vk::instance;
        init_info.PhysicalDevice = vk::physicalDevice;
        init_info.Device = vk::device;
        init_info.QueueFamily = vk::queueFamily;
        init_info.Queue = vk::graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = vk::descriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = vk::allocator;
        init_info.CheckVkResultFn = vk::checkVkResult;
        ImGui_ImplVulkan_Init(&init_info, vk::renderPass);
    }
    ImGui_ImplVulkan_NewFrame();

    RenderHUD::newFrame();
}

void RenderHUD_Vulkan::render()
{
    if (ImGui::GetCurrentContext()) {
        ImGui::Render();
//        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());        
    }
}

}
