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

#include "MessageWindow.h"

#include "global.h" // Global::shared_config
#include "GlobalState.h"
#include "../external/imgui/imgui.h"
#include "TimeHolder.h"

#include <list>
#include <utility>
#include <string>

namespace libtas {

/* Messages to print on screen with the creation time */
static std::list<std::pair<std::string, TimeHolder>> messages;

void MessageWindow::insert(const char* message)
{
    /* Get current time */
    TimeHolder current_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time));

    messages.push_back(std::make_pair(std::string(message), current_time));
}

void MessageWindow::draw()
{
    if (messages.empty()) return;
    
    TimeHolder message_timeout;
    message_timeout = {2, 0};

    /* Get current time */
    TimeHolder current_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x, main_viewport->WorkPos.y + main_viewport->WorkSize.y), ImGuiCond_Always, ImVec2(1.0f, 1.0f));

    if (ImGui::Begin("Messages", nullptr, window_flags)) {
        auto iter = messages.begin();
        while (iter != messages.end()) {
            /* Check if we must remove the message */
            if ((current_time - iter->second) > message_timeout) {
                iter = messages.erase(iter);
            }
            else {
                ImGui::TextUnformatted(iter->first.c_str());
                iter++;
            }
        }
    }
    ImGui::End();
}

void MessageWindow::clear()
{
    messages.clear();
}

}
