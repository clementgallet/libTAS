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

#include "FrameWindow.h"

#include "global.h" // Global::shared_config
#include "../external/imgui/imgui.h"

#include <sstream>

namespace libtas {
    
/* Marker text to print on screen */
static std::string marker;

void FrameWindow::setMarkerText(std::string text)
{
    marker = text;
}

void FrameWindow::draw(uint64_t framecount, uint64_t nondraw_framecount, bool* p_open = nullptr)
{    
    std::string framestr = std::to_string(framecount);
    switch (Global::shared_config.recording) {
    case SharedConfig::RECORDING_READ:
        framestr.append("/").append(std::to_string(Global::shared_config.movie_framecount));
        if (framecount > Global::shared_config.movie_framecount)
            framestr.append(" (Finished)");
        break;
    case SharedConfig::RECORDING_WRITE:
    case SharedConfig::NO_RECORDING:
    default:
        break;
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Framecount", p_open, window_flags))
    {
        ImGui::Text(framestr.c_str());
        if (nondraw_framecount > 0) {
            std::string nondraw_framestr = std::to_string(nondraw_framecount);
            ImGui::TextColored(ImVec4(0.6f, 0.0f, 0.0f, 1.0f), nondraw_framestr.c_str());
        }
        if (!marker.empty())
            ImGui::Text(marker.c_str());
    }
    ImGui::End();
}

}
