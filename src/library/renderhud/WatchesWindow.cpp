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

#include "WatchesWindow.h"

#include "global.h" // Global::shared_config
#include "TimeHolder.h"
#include "../external/imgui/imgui.h"

#include <list>
#include <string>

namespace libtas {

/* Ram watches to print on screen */
static std::list<std::string> watches;

void WatchesWindow::insert(std::string watch)
{
    watches.push_back(watch);
}

void WatchesWindow::reset()
{
    watches.clear();
}

void WatchesWindow::draw(bool* p_open = nullptr)
{
    if (watches.empty()) return;
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x, main_viewport->WorkPos.y), ImGuiCond_Once, ImVec2(1.0f, 0.0f));

    if (ImGui::Begin("RAM Watches", p_open, window_flags)) {
        for (auto iter = watches.begin(); iter != watches.end(); iter++) {
            ImGui::TextUnformatted(iter->c_str());
        }
    }
    ImGui::End();
}

}
