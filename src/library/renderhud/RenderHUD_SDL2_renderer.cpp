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

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_sdlrenderer2.h"

namespace libtas {

DECLARE_ORIG_POINTER_NAMESPACE(SDL_CreateTexture, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_DestroyTexture, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_LockTexture, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_UnlockTexture, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_RenderCopy, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_SetTextureBlendMode, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_GetError, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_GetVersion, SDL2)
DECLARE_ORIG_POINTER_NAMESPACE(SDL_SetWindowResizable, SDL2)

RenderHUD_SDL2_renderer::~RenderHUD_SDL2_renderer()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui::DestroyContext();
}

void RenderHUD_SDL2_renderer::setRenderer(SDL2::SDL_Renderer* r)
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
        if (!orig::SDL_GetVersion)
            return;
        SDL2::SDL_version ver;
        orig::SDL_GetVersion(&ver);
        if ((ver.minor == 0) && (ver.patch < 18))
            return;
        
        if (RenderHUD::init()) {
            ImGui_ImplSDLRenderer2_Init(renderer);
            orig::SDL_SetWindowResizable(sdl::gameSDLWindow, supportsLargerViewport()?SDL_TRUE:SDL_FALSE);
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
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());        
    }
}


}
