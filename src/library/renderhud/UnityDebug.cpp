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

#include "UnityDebug.h"

#include "UnityHacks.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/implot.h"
#include "global.h"

#include <string>

namespace libtas {

void UnityDebug::draw(uint64_t framecount, bool* p_open = nullptr)
{
    static uint64_t old_framecount = 0;
    
    if (ImGui::Begin("Unity Debug", p_open))
    {     
        if (ImPlot::BeginPlot("Job Count", ImVec2(-1,-1), ImPlotFlags_NoTitle)) {
            /* Auto-resize when running, but allow zooming when paused */
            int flags = (framecount != old_framecount) ? (ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit) : 0;
            ImPlot::SetupAxes("Frame","Job Count", 0, flags);
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
            
            const UnityHacks::ScrollingBuffers& unityJobData = UnityHacks::getJobData();

            /* Crop the plot to recent data, because some old threads may have
             * very old data left. */
            float min_x = framecount;

            for (const auto& unityThreadData : unityJobData.Buffers)
                min_x = std::min(min_x, static_cast<float>(framecount - unityThreadData.second.DataX.size()));

            if (framecount != old_framecount)
                ImPlot::SetupAxisLimits(ImAxis_X1, min_x, framecount, ImPlotCond_Always);
            
            for (const auto& unityThreadData : unityJobData.Buffers) {
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
    }
    ImGui::End();
    
    old_framecount = framecount;
}

}
