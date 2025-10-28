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

#include "UnityDebug.h"

#include "checkpoint/ThreadManager.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/implot.h"
#include "global.h"

#include <string>
#include <vector>

namespace libtas {

UnityDebug::ScrollingBuffer::ScrollingBuffer(int max_size = 400) {
    MaxSize = max_size;
    Offset  = 0;
    DataX.reserve(MaxSize);
    DataY.reserve(MaxSize);
}

void UnityDebug::ScrollingBuffer::AddPoint(int x, int y) {
    if (DataX.size() < MaxSize) {
        DataX.push_back(x);
        DataY.push_back(y);
    }
    else {
        DataX[Offset] = x;
        DataY[Offset] = y;
        Offset =  (Offset + 1) % MaxSize;
    }
}
void UnityDebug::ScrollingBuffer::Erase() {
    if (DataX.size() > 0) {
        DataX.clear();
        DataY.clear();
        Offset  = 0;
    }
}

UnityDebug::ScrollingBuffers::ScrollingBuffers() {
    /* Push back the total count */
    Buffers[0] = ScrollingBuffer();
    Buffers[0].name = "Total";
}

void UnityDebug::ScrollingBuffers::AddPoint(float x, float y, int tid) {
    bool new_buffer = (Buffers.find(tid) == Buffers.end());
    Buffers[tid].AddPoint(x, y);
    if (new_buffer) {
        for (ThreadInfo* th = ThreadManager::getThreadList(); th != nullptr; th = th->next) {
            if (th->translated_tid == tid) {
                if (th->state == ThreadInfo::ST_CKPNTHREAD)
                    Buffers[tid].name = "Main";
                else
                    Buffers[tid].name = th->name;
                break;
            }
        }
    }
}

static UnityDebug::ScrollingBuffers jobData;

static std::vector<int> preloadPending;
static std::vector<int> preloadProcessed;

static std::vector<int> preloadAll;

static int preload_pending_count;
static int preload_processed_count;

void UnityDebug::update_preload(int added_count, int processed_count)
{
    preload_pending_count += added_count - processed_count;
    preload_processed_count = processed_count;
}

void UnityDebug::update(uint64_t framecount)
{
    /* Register and reset counts */
    unsigned int unity_job_count = 0;
    for (ThreadInfo* th = ThreadManager::getThreadList(); th != nullptr; th = th->next) {
        if (th->unityJobCount) {
            jobData.AddPoint(framecount, th->unityJobCount, th->translated_tid);
            unity_job_count += th->unityJobCount;
            th->unityJobCount = 0;
        }
    }

    jobData.AddPoint(framecount, unity_job_count, 0);

    /* Preload operations */
    if (preloadPending.size() <= framecount)
        preloadPending.resize(framecount+1);
    if (preloadProcessed.size() <= framecount)
        preloadProcessed.resize(framecount+1);
    
    preloadPending[framecount] += preload_pending_count;
    preloadProcessed[framecount] += preload_processed_count;
    
    /* Build the array that will be used to plot */
    preloadAll.clear();
    int size = 400;
    if (preloadPending.size() < size)
        size = preloadPending.size();

    preloadAll.insert(preloadAll.end(), preloadProcessed.end() - size, preloadProcessed.end());
    preloadAll.insert(preloadAll.end(), preloadPending.end() - size, preloadPending.end());
}

void UnityDebug::draw(uint64_t framecount, bool* p_open = nullptr)
{
    static uint64_t old_framecount = 0;
    
    if (ImGui::Begin("Unity Debug", p_open)) {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("TabBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Job Count")) {
                if (ImPlot::BeginPlot("Job Count", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
                    /* Auto-resize when running, but allow zooming when paused */
                    int flags = (framecount != old_framecount) ? (ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit) : 0;
                    ImPlot::SetupAxes("Frame","Job Count", 0, flags);
                    ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                    ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
                    
                    /* Crop the plot to recent data, because some old threads may have
                     * very old data left. */
                    float min_x = framecount;

                    for (const auto& unityThreadData : jobData.Buffers)
                        min_x = std::min(min_x, static_cast<float>(framecount - unityThreadData.second.DataX.size()));

                    if (framecount != old_framecount)
                        ImPlot::SetupAxisLimits(ImAxis_X1, min_x, framecount, ImPlotCond_Always);
                    
                    for (const auto& unityThreadData : jobData.Buffers) {
                        std::string label = unityThreadData.second.name;
                        if (unityThreadData.first == 0) {
                            ImPlot::PlotShaded(label.c_str(), unityThreadData.second.DataX.data(), unityThreadData.second.DataY.data(), unityThreadData.second.DataY.size(), 0, 0, unityThreadData.second.Offset);
                        }
                        else {
                            label += " (";
                            label += std::to_string(unityThreadData.first);
                            label += ")";                    
                            ImPlot::PlotLine(label.c_str(), unityThreadData.second.DataX.data(), unityThreadData.second.DataY.data(), unityThreadData.second.DataY.size(), 0, unityThreadData.second.Offset);
                        }
                    }            

                    ImPlot::PopStyleVar();    
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Preload")) {
                static const char* labels[] = {"Processed","Pending"};

                if (ImPlot::BeginPlot("Preload", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
                    // ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);

                    /* Auto-resize when running, but allow zooming when paused */
                    int flags = (framecount != old_framecount) ? (ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit) : 0;

                    ImPlot::SetupAxes("Frame","Operations", 0, flags);
                    // ImPlot::SetupAxisTicks(ImAxis_X1,positions, groups, glabels);
                    // ImPlot::SetupAxisTicks(ImAxis_X1, framecount - preloadAll.size() / 2, framecount, preloadAll.size() / 2, nullptr, true);

                    if (framecount != old_framecount)
                        ImPlot::SetupAxisLimits(ImAxis_X1, 1 + framecount - preloadAll.size() / 2, framecount, ImPlotCond_Always);

                    ImPlot::PlotBarGroups(labels, preloadAll.data(), 2, preloadAll.size() / 2, 0.67, 1 + framecount - preloadAll.size() / 2, ImPlotBarGroupsFlags_Stacked);
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    
    old_framecount = framecount;
}

}
