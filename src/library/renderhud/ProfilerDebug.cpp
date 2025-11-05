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

#include <limits>

namespace libtas {

static float timeWindowMs = 1000;
static int timeWindowFrames = 1;
static int windowUnitType = 0;
static TimeHolder currentTime;
static float firstColWidth = 0;

/* Lowest time that a node is registered, for setting the window scroll size */
static TimeHolder minTime = nullTime;

void ProfilerDebug::nodeToPos(const TimeHolder& startTime, const TimeHolder& lengthTime, float available_size, float& lengthTimeMs, float& leftPos, float& lengthPos)
{
    TimeHolder leftTime = startTime - minTime;
    leftPos = (leftTime.toMs() / timeWindowMs) * available_size;

    TimeHolder newLengthTime = lengthTime;
    if (!(newLengthTime != nullTime))
        newLengthTime = currentTime - startTime;
    lengthTimeMs = newLengthTime.toMs();

    lengthPos = (lengthTimeMs / timeWindowMs) * available_size;
}

void ProfilerDebug::renderFrame(int f, const TimeHolder& frame_start, const TimeHolder& frame_end, float available_start, float available_size)
{
    TimeHolder frame_length = frame_end - frame_start;

    float lengthTimeMs, leftPos, lengthPos;
    nodeToPos(frame_start, frame_length, available_size, lengthTimeMs, leftPos, lengthPos);

    /* Add left padding to button, to keep the frame end at the right position  */
    leftPos += 4.0f;
    lengthPos -= 4.0f;

    /* Check if too small */
    if (lengthPos < 1.0f)
        return;

    ImGui::SetCursorPosX(available_start + leftPos);

    char buffer_name[16];
    sprintf(buffer_name, "%d", f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::Button(buffer_name, ImVec2(lengthPos, 0.0f));
    if (ImGui::BeginItemTooltip())
    {
        ImGui::Text("Frame %d in %f ms", f, lengthTimeMs);
        ImGui::EndTooltip();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
}

void ProfilerDebug::renderNode(int nodeId, const Profiler::Database* database, float available_start, float available_size)
{
    const auto& info = database->nodes[nodeId];

    float lengthTimeMs, leftPos, lengthPos;
    nodeToPos(info.startTime, info.lengthTime, available_size, lengthTimeMs, leftPos, lengthPos);

    /* Check if too small or offscreen */
    if (lengthPos < 1.0f)
        return;

    ImGui::SetCursorPosX(available_start + leftPos);

    char buffer_name[16];
    sprintf(buffer_name, "%d", nodeId);

    float hue = info.type * 0.13f;
    float sat_delta = info.depth * -0.2f;
    
    float r, g, b;
    ImGui::ColorConvertHSVtoRGB(hue, 0.6f + sat_delta, 0.6f, r, g, b);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
    ImGui::ColorConvertHSVtoRGB(hue, 0.7f + sat_delta, 0.7f, r, g, b);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r, g, b, 1.0f));
    ImGui::ColorConvertHSVtoRGB(hue, 0.8f + sat_delta, 0.8f, r, g, b);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r, g, b, 1.0f));

    ImGui::Button(info.label.c_str(), ImVec2(lengthPos, 0.0f));
    if (ImGui::BeginItemTooltip())
    {
        ImGui::Text("%s in %f ms", info.label.c_str(), lengthTimeMs);
        if (!info.description.empty())
            ImGui::Text(info.description.c_str());
        ImGui::EndTooltip();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
}

void ProfilerDebug::draw(uint64_t framecount, bool* p_open = nullptr)
{
    if (!ImGui::Begin("Profiler Debug", p_open))
    {
        ImGui::End();
        return;
    }

    static uint64_t old_framecount = 0;
    static uint64_t older_framecount = 0;

    currentTime = Profiler::currentTimeWithoutPause();
    if (minTime == nullTime)
        minTime = currentTime;

    bool updateScroll = false;

    /* Options */
    ImGui::SeparatorText("Options");
    ImGui::BeginChild("Profiler Options", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, 0);

    int pauseFlags = Profiler::getPauseFlags();
    if (ImGui::CheckboxFlags("Hide when Idle", &pauseFlags, Profiler::PAUSE_ON_IDLE))
        Profiler::setPauseFlags(pauseFlags);
    ImGui::SameLine();
    if (ImGui::CheckboxFlags("Hide when Sleep", &pauseFlags, Profiler::PAUSE_ON_SLEEP))
        Profiler::setPauseFlags(pauseFlags);
    
    updateScroll |= ImGui::RadioButton("Units in miliseconds", &windowUnitType, 0);
    ImGui::SameLine();
    updateScroll |= ImGui::RadioButton("Units in frames", &windowUnitType, 1);
    ImGui::SameLine();
    if (windowUnitType == 0)
        updateScroll |= ImGui::SliderFloat("Window size", &timeWindowMs, 1.0f, 10000.0f, "%.1f ms", ImGuiSliderFlags_Logarithmic);
    else {
        updateScroll |= ImGui::SliderInt("Window size", &timeWindowFrames, 1, 100, "%d frames", ImGuiSliderFlags_Logarithmic);

        /* Convert frames to length here */
        const std::vector<TimeHolder>& frameTimings = Profiler::getFrameTimings();
        if (frameTimings.size() < static_cast<unsigned int>(timeWindowFrames))
            timeWindowFrames = frameTimings.size();
        
        if (timeWindowFrames > 0) {
            TimeHolder firstSelectedFrame = *(frameTimings.end() - timeWindowFrames);
            TimeHolder windowLength = currentTime - firstSelectedFrame;
            timeWindowMs = windowLength.toMs();
        }
    }

    ImGui::EndChild();

    ImGui::SeparatorText("Tasks");

    /* We need to specify the size of the table, so that X scrolling will work.
     * We don't know yet what is the size of the first column, so we use the size
     * of the last draw */
    float available_size = ImGui::GetWindowWidth() - firstColWidth;
    TimeHolder fullTime = currentTime - minTime;
    float fullTimeMs = fullTime.toMs();
    float tableWidth = (fullTimeMs / timeWindowMs) * available_size;
    if (fullTimeMs < timeWindowMs)
        tableWidth = available_size;

    if ((old_framecount != older_framecount))
        ImGui::SetNextWindowScroll(ImVec2(tableWidth, -1.0f));
    if (updateScroll) {
        /* Trigger a future scroll update */
        old_framecount = 0;
        older_framecount = 0;
    }

    if (ImGui::BeginTable("Profiler Table", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV, ImVec2(0.0f, 0.0f))) {

        ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tasks", ImGuiTableColumnFlags_WidthFixed, tableWidth);
        ImGui::TableSetupScrollFreeze(1, 1);

        /* We reset the min time to get the new value for the next iteration.
         * we will always be one call behind, which should not matter. */
        TimeHolder futureMinTime = {std::numeric_limits<time_t>::max(),  std::numeric_limits<long>::max()};

        /* Print frames */
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        ImGui::TreeNodeEx("Frame", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen);

        ImGui::TableSetColumnIndex(1);

        float available_start = ImGui::GetCursorPos().x;

        /* Save the width of the first column for the next iteration */
        firstColWidth = available_start;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        const std::vector<TimeHolder>& frameTimings = Profiler::getFrameTimings();
        if (frameTimings.size() > 0) {
            for (long unsigned int f = 0; f < frameTimings.size() - 1; f++) {
                /* Don't print frames that won't show up on screen */
                if (frameTimings[f] < minTime)
                    continue;

                renderFrame(f+1, frameTimings[f], frameTimings[f+1], available_start, available_size);
            }

            /* Render the ongoing frame */
            renderFrame(frameTimings.size(), frameTimings.back(), currentTime, available_start, available_size);
        }

        ImGui::PopStyleVar(1);

        ThreadManager::lockList();

        for (ThreadInfo *thread = ThreadManager::getThreadList(); thread != nullptr; thread = thread->next) {
            
            const std::vector<std::vector<int>>& nodesByDepth = thread->profilerDatabase->populateNodes(futureMinTime);

            if (nodesByDepth.empty() || nodesByDepth[0].empty())
                continue;


            ImGui::TableSetColumnIndex(1);


            // LOG(LL_WARN, LCF_FILEIO, "available_size %f available_start %f", available_size, available_start);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

            bool firstRow = true;
            for (const auto& nodesOneDepth : nodesByDepth) {
                ImGui::TableNextRow();
                
                bool isNodeOpen;
                if (firstRow) {
                    firstRow = false;
                    
                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding;
                    if (nodesByDepth.size() == 1) {
                        // Leaf nodes don't need a collapsing arrow
                        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
                    }

                    ImGui::TableSetColumnIndex(0);
                    
                    // We use the pthread id as a unique identifier for ImGui
                    isNodeOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(thread->pthread_id), nodeFlags, "%s", thread->name.c_str());
                    if (isNodeOpen) {
                        ImGui::TreePop();
                    }
                }
                
                ImGui::TableSetColumnIndex(1);
                
                for (int rootId : nodesOneDepth) {
                    renderNode(rootId, thread->profilerDatabase, available_start, available_size);
                }

                if (!isNodeOpen)
                    break;
            }

            ImGui::PopStyleVar(1);
        }

        ThreadManager::unlockList();
        
        minTime = futureMinTime;
        
        ImGui::EndTable();
    }

    ImGui::End();
    
    older_framecount = old_framecount;
    old_framecount = framecount;
}

}
