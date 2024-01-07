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

#include "AudioDebug.h"

#include "audio/AudioContext.h"
#include "audio/AudioSource.h"
#include "audio/AudioBuffer.h"
#include "../external/imgui/imgui.h"

namespace libtas {

void AudioDebug::draw(bool* p_open = nullptr)
{
    if (!ImGui::Begin("Audio Debug", p_open))
    {
        ImGui::End();
        return;
    }

    const AudioContext& audiocontext = AudioContext::get();
    auto sources = audiocontext.getSourceList();
    const float sampleByUnit = 200.0f;
    
    ImGui::SeparatorText("Active Sources");
    
    /* Compute numbre of sources and minimum of consumed samples to align properly */
    int consumedSamples = 0;
    int activeSources = 0;
    for (const auto& source : sources) {
        if (source->state == AudioSource::SOURCE_PLAYING) {
            activeSources++;
            if (source->getPosition() > consumedSamples)
                consumedSamples = source->getPosition();
        }
    }
    
    /* Compute maximum of non-consumed samples to align properly */
    int nonConsumedSamples = 0;
    for (const auto& source : sources)
    if ((source->queueSize() - source->getPosition()) > nonConsumedSamples)
    nonConsumedSamples = (source->queueSize() - source->getPosition());
    
    ImGui::SetNextWindowScroll(ImVec2((float)consumedSamples / sampleByUnit, 0.0f));
    ImGui::SetNextWindowContentSize(ImVec2((float)(consumedSamples + nonConsumedSamples) / sampleByUnit + ImGui::GetContentRegionAvail().x, 0.0f));
    
    /* Add padding so that we can always scroll so that current source position
    * is at the center */
    float leftPadding = ImGui::GetContentRegionAvail().x/2.0f;
    
    ImGuiStyle& style = ImGui::GetStyle();
    // float child_height = activeSources * ImGui::GetFrameHeightWithSpacing() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
    float child_height = activeSources * ImGui::GetFrameHeightWithSpacing() + style.ScrollbarSize;
            
    if (ImGui::BeginChild("Active Sources", ImVec2(0, child_height), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar)) {
    
        for (const auto& source : sources) {
            if (!(source->state == AudioSource::SOURCE_PLAYING))
                continue;
    
            ImGui::SetCursorPosX(ImGui::GetScrollX());
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Source %d", source->id);
            ImGui::SameLine();
    
            int alignedSamplePos = consumedSamples + leftPadding * sampleByUnit - source->getPosition();
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            for (size_t i = 0; i < source->buffer_queue.size(); i++) {
    
                ImGui::SetCursorPosX((float)alignedSamplePos / sampleByUnit);
    
                const auto& buffer = source->buffer_queue[i];
                char buffer_name[16];
                sprintf(buffer_name, "%d", buffer->id);
                
                float hue = buffer->id * 0.13f;
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.6f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.7f, r, g, b);                
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.8f, r, g, b);                
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r, g, b, 1.0f));
                ImGui::Button(buffer_name, ImVec2((float)buffer->sampleSize / sampleByUnit - 10.0f, 0.0f));
                ImGui::PopStyleColor(3);
                ImGui::SameLine();
    
                alignedSamplePos += buffer->sampleSize;
            }
            ImGui::PopStyleVar(1);
            ImGui::NewLine();
        }
        // ImGui::SetScrollFromPosX(ImGui::GetCursorStartPos().x + (float)consumedSamples/10.0f);
    }
    ImGui::EndChild();
    
    ImGui::SeparatorText("Inactive Sources");

    // child_height = (sources.size() - activeSources) * ImGui::GetFrameHeightWithSpacing() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
    child_height = (sources.size() - activeSources) * ImGui::GetFrameHeightWithSpacing() + style.ScrollbarSize;

    if (ImGui::BeginChild("Inactive Sources", ImVec2(0, child_height), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (const auto& source : sources) {
            if (source->state == AudioSource::SOURCE_PLAYING)
                continue;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Source %2d", source->id);
            ImGui::SameLine();
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            for (size_t i = 0; i < source->buffer_queue.size(); i++) {
                const auto& buffer = source->buffer_queue[i];
                char buffer_name[16];
                sprintf(buffer_name, "%d", buffer->id);

                float hue = buffer->id * 0.13f;
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.6f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.7f, r, g, b);                
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.8f, r, g, b);                
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r, g, b, 1.0f));
                ImGui::Button(buffer_name, ImVec2((float)buffer->sampleSize / sampleByUnit - 10.0f, 0.0f));
                ImGui::PopStyleColor(3);
                ImGui::SameLine();
            }
            ImGui::PopStyleVar(1);
            ImGui::NewLine();
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

}
