/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ProfilerDebug.h"
#include "Profiler.h"
#include "logging.h"

#include "checkpoint/ThreadManager.h"
#include "../external/imgui/imgui.h"
#include "global.h"

namespace libtas {

static float timeWindowMs = 1000;
static int timeWindowFrames = 1;
static int windowUnitType = 0;
static TimeHolder currentTime;

void ProfilerDebug::nodeToPos(const Profiler::ScopeInfo& info, float& leftPos, float& lengthPos, float available_size)
{
    TimeHolder leftTime = currentTime - info.startTime;

    leftPos = (1 - (leftTime.toMs() / timeWindowMs)) * available_size;
    if (info.lengthTime != nullTime)
        lengthPos = (info.lengthTime.toMs() / timeWindowMs) * available_size;
    else
        lengthPos = (leftTime.toMs() / timeWindowMs) * available_size;
        
    if (leftPos < 0) {
        lengthPos += leftPos;
        leftPos = 0;
    }
}

void ProfilerDebug::renderNode(int nodeId, const Profiler::Database* database, float available_start, float available_size)
{
    const auto& info = database->nodes[nodeId];

    float leftPos, lengthPos;
    nodeToPos(info, leftPos, lengthPos, available_size);

    /* Check if offscreen */
    if (lengthPos < 0)
        return;

    if ((leftPos + lengthPos) < 0)
        return;

    ImGui::SetCursorPosX(available_start + leftPos);

    char buffer_name[16];
    sprintf(buffer_name, "%d", nodeId);

    ImGui::Button(buffer_name, ImVec2(lengthPos, 0.0f));
    ImGui::SameLine();
}

void ProfilerDebug::draw(bool* p_open = nullptr)
{
    if (!ImGui::Begin("Profiler Debug", p_open))
    {
        ImGui::End();
        return;
    }

    currentTime = Profiler::currentTimeWithoutPause();

    /* Options */
    ImGui::BeginChild("Profiler Options", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, 0);

    ImGui::RadioButton("Units in miliseconds", &windowUnitType, 0); ImGui::SameLine();
    ImGui::RadioButton("Units in frames", &windowUnitType, 1); ImGui::SameLine();
    if (windowUnitType == 0)
        ImGui::SliderFloat("Window size", &timeWindowMs, 1.0f, 10000.0f, "%.1f ms", ImGuiSliderFlags_Logarithmic);
    else
        ImGui::SliderInt("Window size", &timeWindowFrames, 1, 100, "%d frames", ImGuiSliderFlags_Logarithmic);

    ImGui::EndChild();

    ImGui::BeginChild("Profiler Tasks");

    if (ImGui::BeginTable("Profiler Table", 2, ImGuiTableFlags_ScrollY)) {

        ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tasks", ImGuiTableColumnFlags_WidthStretch);
        
        ThreadManager::lockList();

        for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
            
            std::vector<std::vector<int>> nodesByDepth;
            
            bool hasChildren = thread->profilerDatabase->populateNodes(nodesByDepth);

            if (nodesByDepth.empty() || nodesByDepth[0].empty())
                continue;

            ImGui::TableNextRow();

            ImGuiTreeNodeFlags nodeFlags = 0;
            if (!hasChildren) {
                // Leaf nodes don't need a collapsing arrow and don't push to the ID stack
                nodeFlags |= ImGuiTreeNodeFlags_Leaf;
            }

            ImGui::TableSetColumnIndex(0);
            
            ImGui::AlignTextToFramePadding();
            // We use the pthread id as a unique identifier for ImGui
            const bool isNodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(thread->pthread_id), nodeFlags, "%s", thread->name.c_str());
            if (isNodeOpen) {
                ImGui::TreePop();
            }

            ImGui::TableSetColumnIndex(1);

            float available_size = ImGui::GetContentRegionAvail().x;
            float available_start = ImGui::GetCursorPos().x;

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

            for (const auto& nodesOneDepth : nodesByDepth) {
                for (int rootId : nodesOneDepth) {
                    renderNode(rootId, thread->profilerDatabase, available_start, available_size);
                }

                if (!isNodeOpen)
                    break;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(1);
            }

            ImGui::PopStyleVar(1);

        }

        ThreadManager::unlockList();
        ImGui::EndTable();
    }

    ImGui::EndChild();
    ImGui::End();
}

}
