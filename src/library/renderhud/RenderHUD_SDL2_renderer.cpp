/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "RenderHUD_SDL2_renderer.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"

#include "sdl/sdlwindows.h"
#include "sdl/sdldynapi.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_sdlrenderer2.h"

namespace libtas {

RenderHUD_SDL2_renderer::~RenderHUD_SDL2_renderer()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui::DestroyContext();
}

void RenderHUD_SDL2_renderer::setRenderer(sdl2::SDL_Renderer* r)
{
    /* If renderer has changed, the texture becomes invalid */
    if (renderer && (renderer != r)) {
        ImGui_ImplSDLRenderer2_Shutdown();        
        ImGui_ImplSDLRenderer2_Init(r);
    }
    renderer = r;
}

void RenderHUD_SDL2_renderer::newFrame()
{
    if (!ImGui::GetCurrentContext()) {
        if (!renderer)
            return;
        
        /* 2.0.18 is required for OSD */    
        if (!ORIG_SDL2_FUNCTION_POINTER(SDL_GetVersion))
            return;
        sdl2::SDL_version ver;
        ORIG_SDL2_CALL(SDL_GetVersion, (&ver));
        if ((ver.minor == 0) && (ver.patch < 18))
            return;
        
        if (RenderHUD::init()) {
            ImGui_ImplSDLRenderer2_Init(renderer);
            ORIG_SDL2_CALL(SDL_SetWindowResizable, (sdl::gameSDLWindow, supportsLargerViewport()?SDL_TRUE:SDL_FALSE));
        }
        else
            return;
    }
    ImGui_ImplSDLRenderer2_NewFrame();

    RenderHUD::newFrame();
}

void RenderHUD_SDL2_renderer::render()
{
    if (ImGui::GetCurrentContext()) {
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);        
    }
}


}
