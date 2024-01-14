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

#include "RenderHUD.h"
#include "Crosshair.h"
#include "FrameWindow.h"
#include "InputsWindow.h"
#include "LogWindow.h"
#include "LuaDraw.h"
#include "MessageWindow.h"
#include "WatchesWindow.h"
#include "AudioDebug.h"

#include "GlobalState.h"
#include "global.h" // Global::shared_config
#include "xlib/xdisplay.h" // x11::gameDisplays
#include "xlib/xwindows.h" // x11::gameXWindows
#include "FPSMonitor.h"
#include "screencapture/ScreenCapture.h"
#include "general/timewrappers.h" // clock_gettime
#include "TimeHolder.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_xlib.h"

#include <sstream>

namespace libtas {

bool RenderHUD::init()
{
    if (!ImGui::GetCurrentContext()) {
        if (x11::gameXWindows.empty())
            return false;
        
        /* TODO: select one display? */
        for (int i=0; i<GAMEDISPLAYNUM; i++) {
            if (x11::gameDisplays[i]) {
                ImGui::CreateContext();
                GlobalNative gn;
                ImGui_ImplXlib_Init(x11::gameDisplays[i], x11::gameXWindows.front());
                return true;
            }
        }
    }
    return false;
}

void RenderHUD::newFrame()
{
    if (!ImGui::GetCurrentContext())
        return;

    if (framesBeforeIdle > 0)
        framesBeforeIdle--;

    GlobalNative gn;
    ImGui_ImplXlib_NewFrame();
    ImGui::NewFrame();
}

void RenderHUD::endFrame()
{
    if (!ImGui::GetCurrentContext())
        return;

    ImGui::EndFrame();
}

void RenderHUD::drawAll(uint64_t framecount, uint64_t nondraw_framecount, const AllInputs& ai, const AllInputs& preview_ai)
{
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    /* Show game window if backend supports it */
    /* Must be placed **before** offering the option to enable/disable it */
    if (renderGameWindow() && ScreenCapture::isInited()) {
        /* Create the window that will hold the game texture */
        int w = 0, h = 0;
        ScreenCapture::getDimensions(w, h);

        /* Remove padding so that the texture is aligned with the window */
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(w, h));
        if (ImGui::Begin("Game window", &show_game_window, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize)) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(ScreenCapture::screenTexture()), 
                ImVec2(pos.x, pos.y), 
                ImVec2(pos.x + w, pos.y + h), 
                ImVec2(0, invertedOrigin()?1:0), 
                ImVec2(1, invertedOrigin()?0:1)
            );                
        }
        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    static bool show_framecount = true;
    static bool show_inputs = true;
    static bool show_messages = true;
    static bool show_watches = true;
    static bool show_lua = true;
    static bool show_crosshair = false;
    static bool show_log = false;
    static bool show_audio = false;
    static bool show_demo = false;
    
    if (Global::shared_config.osd) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Display")) {
                ImGui::MenuItem("Detach game window", nullptr, &show_game_window, supportsGameWindow());
                ImGui::Separator();
                ImGui::MenuItem("Frame", nullptr, &show_framecount);
                ImGui::MenuItem("Inputs", nullptr, &show_inputs);
                ImGui::MenuItem("Messages", nullptr, &show_messages);
                ImGui::MenuItem("Watches", nullptr, &show_watches);
                ImGui::MenuItem("Lua", nullptr, &show_lua);
                ImGui::MenuItem("Crosshair", nullptr, &show_crosshair);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")) {
                ImGui::MenuItem("Log", nullptr, &show_log);
                ImGui::MenuItem("Audio", nullptr, &show_audio);
                ImGui::MenuItem("Demo", nullptr, &show_demo);
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            float fps = FPSMonitor::tickRedraw();
            ImGui::Text("FPS: %2.1f", fps);
            ImGui::EndMainMenuBar();
        }        
    }
    
    if (show_framecount)
        FrameWindow::draw(framecount, nondraw_framecount, &show_framecount);

    if (show_inputs)
        InputsWindow::draw(ai, preview_ai, &show_inputs);
        
    if (show_messages)
        MessageWindow::draw();
    else
        MessageWindow::clear();

    if (show_watches)
        WatchesWindow::draw(&show_watches);

    if (show_lua)
        LuaDraw::draw();

    if (show_crosshair)
        Crosshair::draw(ai);
        
    if (show_log)
        LogWindow::draw(&show_log);

    if (show_audio)
        AudioDebug::draw(framecount, &show_audio);

    if (show_demo)
        ImGui::ShowDemoWindow(&show_demo);
}

int RenderHUD::framesBeforeIdle = 0;

void RenderHUD::userInputs()
{
    /* The more complex ImGui interactions need 2-3 frames to operate */
    framesBeforeIdle = 3;
}

bool RenderHUD::doRender()
{
    if (framesBeforeIdle > 0)
        return true;
        
    /* Idling with 100ms steps between renders */
    static const TimeHolder stepTime({0, 100000000});
    static TimeHolder lastTime{}; // -> member, update on render
    
    if (lastTime.tv_sec == 0) {
        NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &lastTime));
        return true;
    }

    TimeHolder currentTime;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &currentTime));
    
    TimeHolder deltaTime = currentTime - lastTime;
    if (deltaTime > stepTime) {
        lastTime = currentTime;
        return true;
    }
    return false;
}

/* Returns if the game is rendered inside an ImGui window */
bool RenderHUD::renderGameWindow()
{
    if (!supportsGameWindow())
        return false;
        
    return show_game_window;    
}

}
