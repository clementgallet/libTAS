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

#include "RenderHUD_GL.h"

#include "logging.h"
#include "hook.h"
#include "ScreenCapture.h"
#include "GlobalState.h"
#include "sdl/sdlwindows.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_opengl3.h"

namespace libtas {

RenderHUD_GL::~RenderHUD_GL() {
    fini();
}

void RenderHUD_GL::fini() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void RenderHUD_GL::newFrame()
{
    if (!ImGui::GetCurrentContext()) {
        ImGui::CreateContext();
        ImGui_ImplOpenGL3_Init("#version 130");
    }
    ImGui_ImplOpenGL3_NewFrame();
    updateCursor();

    /* Check on each frame to accomodate for window resizing */
    int width, height;
    ScreenCapture::getDimensions(width, height);
    
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1, 1);

    static TimeHolder oldTime;
    TimeHolder currentTime;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &currentTime));
    TimeHolder deltaTime = currentTime - oldTime;
    io.DeltaTime = (oldTime.tv_sec == 0) ? 1.0f / 60.0f : (float) deltaTime.tv_sec + ((float) deltaTime.tv_nsec) / 1000000000.0f;
    oldTime = currentTime;

    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
}

void RenderHUD_GL::render()
{
    if (ImGui::GetCurrentContext()) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());        
    }
}

}
