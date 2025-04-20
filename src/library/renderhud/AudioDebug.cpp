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

#include "AudioDebug.h"

#include "audio/AudioContext.h"
#include "audio/AudioSource.h"
#include "audio/AudioBuffer.h"
#include "../external/imgui/imgui.h"
#include "global.h"

#include <cinttypes>
#include <cmath>

namespace libtas {

__attribute__ ((unused)) static const char* formatToString(AudioBuffer::SampleFormat format)
{
    switch (format) {
        case AudioBuffer::SAMPLE_FMT_U8:
            return "u8";
        case AudioBuffer::SAMPLE_FMT_S16:
            return "s16";
        case AudioBuffer::SAMPLE_FMT_S32:
            return "s32";
        case AudioBuffer::SAMPLE_FMT_FLT:
            return "float";
        case AudioBuffer::SAMPLE_FMT_DBL:
            return "double";
        case AudioBuffer::SAMPLE_FMT_MSADPCM:
            return "MS-ADPCM";
        default:
            return "unknown";
    }
    return "unknown";
}

void AudioDebug::draw(uint64_t framecount, bool* p_open = nullptr)
{
    if (!ImGui::Begin("Audio Debug", p_open))
    {
        ImGui::End();
        return;
    }

    const AudioContext& audiocontext = AudioContext::get();
    auto sources = audiocontext.getSourceList();
    
    ImGui::SeparatorText("Active Sources");
    
    /* Compute number of sources and maximum of consumed and non-consumed
     * miliseconds to align properly */
    float consumedMS = 0;
    float nonConsumedMS = 0;
    float minBufferMS = 1000000.0f;
    int activeSources = 0;
    for (const auto& source : sources) {
        if (source->state == AudioSource::SOURCE_PLAYING) {
            activeSources++;
            if (source->buffer_queue.size() != 0) {
                float ms = 1000.0f * (float)source->getPosition() / (float)source->buffer_queue[0]->frequency;
                if (ms > consumedMS)
                    consumedMS = ms;

                ms = 1000.0f * (float)(source->queueSize() - source->getPosition()) / (float)source->buffer_queue[0]->frequency;
                if (ms > nonConsumedMS)
                    nonConsumedMS = ms;
                    
                for (size_t i = 0; i < source->buffer_queue.size(); i++) {
                    const auto& buffer = source->buffer_queue[i];
                    float bufferMS = 1000.0f * (float)buffer->sampleSize / (float)buffer->frequency;
                    minBufferMS = std::min(minBufferMS, bufferMS);
                }
            }
        }
    }
    
    /* Determine the level of zoom depending on the buffer size */
    float msPerUnit = (minBufferMS != 1000000.0f) ? (minBufferMS / 25.0f) : 10.0f;
    
    /* Get ranges to avoid potential zoom/dezoom */
    msPerUnit = std::scalbn(1.0, std::ilogb(msPerUnit));

    int stepFrame = msPerUnit * 16;

    ImGui::SetNextWindowScroll(ImVec2(consumedMS / msPerUnit, 0.0f));
    ImGui::SetNextWindowContentSize(ImVec2((consumedMS + nonConsumedMS) / msPerUnit + ImGui::GetContentRegionAvail().x, 0.0f));
    
    /* Add padding so that we can always scroll so that current source position
    * is at the center */
    float leftPadding = ImGui::GetContentRegionAvail().x/2.0f;
    
    ImGuiStyle& style = ImGui::GetStyle();
    // float child_height = activeSources * ImGui::GetFrameHeightWithSpacing() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
    float child_height = (activeSources+1) * ImGui::GetFrameHeightWithSpacing();
    
    if (ImGui::BeginChild("Active Sources", ImVec2(0, child_height), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None)) {
    
        /* Print frame lines */
        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        /* I am supposed to write the following line, but setting the window scroll
         * can have some rounding difference, that would make the line oscillating
         * by one pixel. Reading from the actual scrolled value remove the oscillation */
        //float centerPosX = (float)consumedMS / msPerUnit + leftPadding;
        float centerPosX = ImGui::GetScrollX() + leftPadding;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x+centerPosX, pos.y), ImVec2(pos.x+centerPosX, pos.y+child_height), 0x80ffffff, 1.0f);
        
        float sizeMS = ImGui::GetWindowSize().x * msPerUnit;
        int sizeFrame = (sizeMS * Global::shared_config.initial_framerate_num) / (1000 * Global::shared_config.initial_framerate_den);
        
        int64_t firstFrame = framecount - (sizeFrame / 2);
        int64_t lastFrame = framecount + (sizeFrame / 2);
        firstFrame = firstFrame - (firstFrame % stepFrame);
        if (firstFrame < 0) firstFrame = 0;
        
        for (int64_t f = firstFrame; f < lastFrame; f += stepFrame) {
            float framePos = centerPosX + (f - (int64_t)framecount) * 1000 * Global::shared_config.initial_framerate_den / (Global::shared_config.initial_framerate_num * msPerUnit);
            ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x+framePos, pos.y), ImVec2(pos.x+framePos, pos.y+ImGui::GetTextLineHeight()), 0x80ffffff, 1.0f);
            char frame_string[21];
            sprintf(frame_string, "%" PRId64, f);
            ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x+framePos+5.0f, pos.y), 0x80ffffff, frame_string, nullptr);
        }
        
        ImGui::NewLine();

        for (const auto& source : sources) {
            if (!(source->state == AudioSource::SOURCE_PLAYING))
                continue;
            
            ImGui::SetCursorPosX(ImGui::GetScrollX());
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Source %d", source->id);
            
            if (source->buffer_queue.size() == 0)
                continue;
            
            ImGui::SameLine();
    
            int alignedSamplePos = consumedMS + leftPadding * msPerUnit - (1000.0f * (float)source->getPosition() / (float)source->buffer_queue[0]->frequency);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            for (size_t i = 0; i < source->buffer_queue.size(); i++) {
    
                ImGui::SetCursorPosX((float)alignedSamplePos / msPerUnit);
    
                const auto& buffer = source->buffer_queue[i];
                char buffer_name[16];
                sprintf(buffer_name, "%d", buffer->id);
                
                float bufferMS = 1000.0f * (float)buffer->sampleSize / (float)buffer->frequency;
                
                float hue = buffer->id * 0.13f;
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.6f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.7f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.8f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r, g, b, 1.0f));
                ImGui::Button(buffer_name, ImVec2(std::max(bufferMS / msPerUnit, 8.0f) - 5.0f, 0.0f));
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Id: %d", buffer->id);
                    ImGui::Text("Sample size: %d", buffer->sampleSize);
                    ImGui::Text("Length: %f sec", (float)buffer->sampleSize / (float)buffer->frequency);
                    ImGui::EndTooltip();
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine();
    
                alignedSamplePos += bufferMS;
            }
            ImGui::PopStyleVar(1);
            ImGui::NewLine();
        }
    }
    ImGui::EndChild();
    
    ImGui::SeparatorText("Inactive Sources");

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

                float bufferMS = 1000.0f * (float)buffer->sampleSize / (float)buffer->frequency;

                float hue = buffer->id * 0.13f;
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.6f, 0.6f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.7f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r, g, b, 1.0f));
                ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 0.8f, r, g, b);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r, g, b, 1.0f));
                ImGui::Button(buffer_name, ImVec2(std::max(bufferMS / msPerUnit, 8.0f) - 5.0f, 0.0f));
                if (ImGui::BeginItemTooltip())
                {
                    ImGui::Text("Id: %d", buffer->id);
                    ImGui::Text("Sample size: %d", buffer->sampleSize);
                    ImGui::Text("Length: %f sec", (float)buffer->sampleSize / (float)buffer->frequency);
                    ImGui::EndTooltip();
                }
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
