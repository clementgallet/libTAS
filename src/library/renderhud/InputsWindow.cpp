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

#include "InputsWindow.h"

#include "global.h" // Global::shared_config
#include "../external/keysymdesc.h"
#include "../external/imgui/imgui.h"

#include <sstream>

namespace libtas {

void InputsWindow::draw(const AllInputsFlat& ai, const AllInputsFlat& preview_ai, bool* p_open = nullptr)
{
    std::string inputs_str = formatInputs(ai);
    std::string preview_inputs_str = formatInputs(preview_ai);
    
    if (!inputs_str.empty() || !preview_inputs_str.empty()) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + main_viewport->WorkSize.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));

        /* Guess and set the window size, so that it appears at correct size
         * on first draw */
        const ImVec2 preview_size = ImGui::CalcTextSize(preview_inputs_str.c_str());
        const ImVec2 size = ImGui::CalcTextSize(inputs_str.c_str());
        float max_width = std::max(preview_size.x, size.x);
        int line_count = 2;
        if (preview_inputs_str.empty()) line_count--;
        if (inputs_str.empty()) line_count--;
        ImGui::SetNextWindowContentSize(ImVec2(max_width, line_count*ImGui::GetTextLineHeight()));

        if (ImGui::Begin("Inputs", nullptr, window_flags))
        {
            if (!preview_inputs_str.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", preview_inputs_str.c_str());
            }
            if (!inputs_str.empty()) {
                ImGui::TextUnformatted(inputs_str.c_str());
            }
        }
        ImGui::End();
    }
}

std::string InputsWindow::formatInputs(const AllInputsFlat& ai)
{
    std::ostringstream oss;

    /* Flags */
    if (ai.misc.flags & (1 << SingleInput::FLAG_RESTART)) {
        oss << "[Restart] ";
    }
    for (int i=0; i<4; i++) {
        if (ai.misc.flags & (1 << (SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED+i))) {
            oss << "[J" << i << " added/removed] ";
        }
    }
    if (ai.misc.flags & (1 << SingleInput::FLAG_FOCUS_UNFOCUS)) {
        oss << "[Un/Focus] ";
    }

    /* Keyboard */
#ifdef __unix__
    for (int i=0; i<AllInputs::MAXKEYS; i++) {
        if (ai.keyboard[i]) {
            const char* str = KEYSYM_TO_DESC(ai.keyboard[i]);
            if (str)
                oss << "[K " << str << "] ";
            else
                oss << "[K ???] ";
        }
    }
#else
    /* TODO! */
#endif

    /* Mouse */
    if (Global::shared_config.mouse_support) {
        if (ai.pointer.x != 0 || ai.pointer.y != 0 || ai.pointer.wheel != 0) {
            oss << "[M " << ai.pointer.x << ":" << ai.pointer.y << ":" << ai.pointer.wheel << ":";
            oss << ((ai.pointer.mode==SingleInput::POINTER_MODE_RELATIVE)?"R":"A");
            oss << "] ";
        }
        if (ai.pointer.mask & (1 << SingleInput::POINTER_B1))
            oss << "[M b1] ";
        if (ai.pointer.mask & (1 << SingleInput::POINTER_B2))
            oss << "[M b2] ";
        if (ai.pointer.mask & (1 << SingleInput::POINTER_B3))
            oss << "[M b3] ";
        if (ai.pointer.mask & (1 << SingleInput::POINTER_B4))
            oss << "[M b4] ";
        if (ai.pointer.mask & (1 << SingleInput::POINTER_B5))
            oss << "[M b5] ";
    }

    /* Joystick */
    for (int i=0; i<Global::shared_config.nb_controllers; i++) {
        for (int j=0; j<ControllerInputs::MAXAXES; j++) {
            if (ai.controllers[i].axes[j] != 0)
                oss << "[J" << i << " a" << j << ":" << ai.controllers[i].axes[j] << "] ";
        }

        for (int j=0; j<16; j++) {
            if (ai.controllers[i].buttons & (1 << j))
                oss << "[J" << i << " b" << j << "] ";
        }
    }

    return oss.str();
}

}
