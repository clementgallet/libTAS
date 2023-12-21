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

#include "logging.h"
#include "hook.h"
#include "global.h" // Global::shared_config
#include "ScreenCapture.h"
#include "GlobalState.h"
#include "../external/keysymdesc.h"
#include "../external/imgui/imgui.h"

#include <sstream>

namespace libtas {

std::list<std::pair<std::string, TimeHolder>> RenderHUD::messages;
std::list<std::string> RenderHUD::watches;
std::list<std::unique_ptr<RenderHUD::LuaShape>> RenderHUD::lua_shapes;
std::string RenderHUD::marker;

void RenderHUD::LuaText::render(RenderHUD *hud)
{
    hud->renderText(text.c_str(), fg_color, bg_color, x, y);
}

void RenderHUD::LuaPixel::render(RenderHUD *hud)
{
    hud->renderPixel(x, y, color);
}

void RenderHUD::LuaRect::render(RenderHUD *hud)
{
    hud->renderRect(x, y, w, h, thickness, outline, fill);
}

void RenderHUD::LuaLine::render(RenderHUD *hud)
{
    hud->renderLine(x0, y0, x1, y1, color);
}

void RenderHUD::LuaEllipse::render(RenderHUD *hud)
{
    hud->renderEllipse(center_x, center_y, radius_x, radius_y, color);
}

void RenderHUD::renderPixel(int x, int y, Color color)
{
    std::unique_ptr<SurfaceARGB> surf(new SurfaceARGB(1, 1));
    surf->fill(color);
    renderSurface(std::move(surf), x, y);        
}

void RenderHUD::renderRect(int x, int y, int w, int h, int t, Color outline_color, Color fill_color)
{
    std::unique_ptr<SurfaceARGB> surf(new SurfaceARGB(w, h));
    surf->fill(fill_color);
    surf->fillBorder(outline_color, t);
    renderSurface(std::move(surf), x, y);
}

void RenderHUD::renderLine(int x0, int y0, int x1, int y1, Color color)
{
    std::unique_ptr<SurfaceARGB> surf(new SurfaceARGB(std::abs(x1-x0)+1, std::abs(y1-y0)+1));
    surf->drawLine(color, ((x1-x0)*(y1-y0)) > 0);
    renderSurface(std::move(surf), std::min(x0, x1), std::min(y0, y1));
}

void RenderHUD::renderEllipse(int center_x, int center_y, int radius_x, int radius_y, Color color)
{
    std::unique_ptr<SurfaceARGB> surf(new SurfaceARGB(2*radius_x+1, 2*radius_y+1));
    surf->drawEllipse(color);
    renderSurface(std::move(surf), center_x-radius_x, center_y-radius_y);
}

void RenderHUD::locationToCoords(int location, int& x, int& y)
{
    int width, height;
    ScreenCapture::getDimensions(width, height);

    if (location & SharedConfig::OSD_LEFT)         x = 5;
    else if (location & SharedConfig::OSD_HCENTER) x = width / 2;
    else                                           x = width - 30;

    if (location & SharedConfig::OSD_TOP)          y = 5;
    else if (location & SharedConfig::OSD_VCENTER) y = height / 2;
    else                                           y = height - 30;

    /* Add an offset if other text was displayed here */
    int offset_index = 0;
    if (location & SharedConfig::OSD_HCENTER)     offset_index += 1;
    else if (location & SharedConfig::OSD_RIGHT)  offset_index += 2;
    if (location & SharedConfig::OSD_VCENTER)     offset_index += 3;
    else if (location & SharedConfig::OSD_BOTTOM) offset_index += 6;

    y += offsets[offset_index];
    if (location & SharedConfig::OSD_BOTTOM)
        offsets[offset_index] -= 25;
    else
        offsets[offset_index] += 25;
}

void RenderHUD::resetOffsets()
{
    memset(offsets, 0, 9 * sizeof(int));
}

void RenderHUD::drawFrame(uint64_t framecount, uint64_t nondraw_framecount)
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
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos, ImGuiCond_Once, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Framecount", nullptr, window_flags))
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

void RenderHUD::drawInputs(const AllInputs& ai, const AllInputs& preview_ai)
{
    std::string inputs_str = formatInputs(ai);
    std::string preview_inputs_str = formatInputs(preview_ai);
    
    if (!inputs_str.empty() || !preview_inputs_str.empty()) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + main_viewport->WorkSize.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        if (ImGui::Begin("Inputs", nullptr, window_flags))
        {
            if (!preview_inputs_str.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), preview_inputs_str.c_str());
            }
            if (!inputs_str.empty()) {
                ImGui::Text(inputs_str.c_str());
            }
        }
        ImGui::End();
    }
}

std::string RenderHUD::formatInputs(const AllInputs& ai)
{
    std::ostringstream oss;

    /* Flags */
    if (ai.flags & (1 << SingleInput::FLAG_RESTART)) {
        oss << "[Restart] ";
    }
    for (int i=0; i<4; i++) {
        if (ai.flags & (1 << (SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED+i))) {
            oss << "[J" << i << " added/removed] ";
        }
    }
    if (ai.flags & (1 << SingleInput::FLAG_FOCUS_UNFOCUS)) {
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
        if (ai.pointer_x != -1) {
            oss << "[M " << ai.pointer_x << ":" << ai.pointer_y << ":";
            oss << ((ai.pointer_mode==SingleInput::POINTER_MODE_RELATIVE)?"R":"A");
            oss << "] ";
        }
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B1))
            oss << "[M b1] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B2))
            oss << "[M b2] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B3))
            oss << "[M b3] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B4))
            oss << "[M b4] ";
        if (ai.pointer_mask & (1 << SingleInput::POINTER_B5))
            oss << "[M b5] ";
    }

    /* Joystick */
    for (int i=0; i<Global::shared_config.nb_controllers; i++) {
        if (!ai.controllers[i])
            continue;

        for (int j=0; j<ControllerInputs::MAXAXES; j++) {
            if (ai.controllers[i]->axes[j] != 0)
                oss << "[J" << i << " a" << j << ":" << ai.controllers[i]->axes[j] << "] ";
        }

        for (int j=0; j<16; j++) {
            if (ai.controllers[i]->buttons & (1 << j))
                oss << "[J" << i << " b" << j << "] ";
        }
    }

    return oss.str();
}

void RenderHUD::insertMessage(const char* message)
{
    /* Get current time */
    TimeHolder current_time;
    NATIVECALL(clock_gettime(CLOCK_MONOTONIC, &current_time));

    messages.push_back(std::make_pair(std::string(message), current_time));
}

void RenderHUD::drawMessages()
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
                ImGui::Text(iter->first.c_str());
                iter++;
            }
        }
    }
    ImGui::End();
}

void RenderHUD::insertWatch(std::string watch)
{
    watches.push_back(watch);
}

void RenderHUD::resetWatches()
{
    watches.clear();
}

void RenderHUD::drawWatches()
{
    if (watches.empty()) return;
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x, main_viewport->WorkPos.y), ImGuiCond_Once, ImVec2(1.0f, 0.0f));

    if (ImGui::Begin("RAM Watches", nullptr, window_flags)) {
        for (auto iter = watches.begin(); iter != watches.end(); iter++) {
            ImGui::Text(iter->c_str());
        }
    }
    ImGui::End();
}

void RenderHUD::drawLua()
{
    for (auto &shape : lua_shapes) {
        shape->render(this);
    }
}

void RenderHUD::setMarkerText(std::string text)
{
    marker = text;
}

void RenderHUD::drawCrosshair(const AllInputs& ai)
{
    int size = 5;
    renderRect(ai.pointer_x-1, ai.pointer_y-size-1, 3, 2*size+3, 0, {0, 0, 0, 255}, {0, 0, 0, 255});
    renderRect(ai.pointer_x-size-1, ai.pointer_y-1, 2*size+3, 3, 0, {0, 0, 0, 255}, {0, 0, 0, 255});
    renderLine(ai.pointer_x, ai.pointer_y-size, ai.pointer_x, ai.pointer_y+size, {255, 255, 255, 255});
    renderLine(ai.pointer_x-size, ai.pointer_y, ai.pointer_x+size, ai.pointer_y, {255, 255, 255, 255});    
}

void RenderHUD::insertLuaText(int x, int y, std::string text, uint32_t fg_color, uint32_t bg_color)
{
    auto lt = new LuaText();
    lt->x = x;
    lt->y = y;
    lt->text = text;
    lt->fg_color = {static_cast<uint8_t>((fg_color >> 16) & 0xff),
                    static_cast<uint8_t>((fg_color >> 8) & 0xff),
                    static_cast<uint8_t>(fg_color & 0xff),
                    static_cast<uint8_t>((fg_color >> 24) & 0xff)};
    lt->bg_color = {static_cast<uint8_t>((bg_color >> 16) & 0xff),
                    static_cast<uint8_t>((bg_color >> 8) & 0xff),
                    static_cast<uint8_t>(bg_color & 0xff),
                    static_cast<uint8_t>((bg_color >> 24) & 0xff)};
    lua_shapes.emplace_back(lt);
}

void RenderHUD::insertLuaPixel(int x, int y, uint32_t color)
{
    auto lp = new LuaPixel;
    lp->x = x;
    lp->y = y;
    lp->color = {static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff)};
    lua_shapes.emplace_back(lp);
}

void RenderHUD::insertLuaRect(int x, int y, int w, int h, int thickness, uint32_t outline, uint32_t fill)
{
    auto lr = new LuaRect();
    lr->x = x;
    lr->y = y;
    lr->w = w;
    lr->h = h;
    lr->thickness = thickness;
    lr->outline = {static_cast<uint8_t>((outline >> 16) & 0xff),
                   static_cast<uint8_t>((outline >> 8) & 0xff),
                   static_cast<uint8_t>(outline & 0xff),
                   static_cast<uint8_t>((outline >> 24) & 0xff)};
    lr->fill = {static_cast<uint8_t>((fill >> 16) & 0xff),
                static_cast<uint8_t>((fill >> 8) & 0xff),
                static_cast<uint8_t>(fill & 0xff),
                static_cast<uint8_t>((fill >> 24) & 0xff)};
    lua_shapes.emplace_back(lr);
}

void RenderHUD::insertLuaLine(int x0, int y0, int x1, int y1, uint32_t color)
{
    auto ll = new LuaLine();
    ll->x0 = x0;
    ll->y0 = y0;
    ll->x1 = x1;
    ll->y1 = y1;
    ll->color = {static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff)};
    lua_shapes.emplace_back(ll);
}

void RenderHUD::insertLuaEllipse(int center_x, int center_y, int radius_x, int radius_y, uint32_t color)
{
    auto le = new LuaEllipse();
    le->center_x = center_x;
    le->center_y = center_y;
    le->radius_x = radius_x;
    le->radius_y = radius_y;
    le->color = {static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff)};
    lua_shapes.emplace_back(le);
}

void RenderHUD::resetLua()
{
    lua_shapes.clear();
}

void RenderHUD::drawAll(uint64_t framecount, uint64_t nondraw_framecount, const AllInputs& ai, const AllInputs& preview_ai)
{
    if (!ImGui::GetCurrentContext()) {
        return;
    }

    if (Global::shared_config.osd & SharedConfig::OSD_FRAMECOUNT)
        drawFrame(framecount, nondraw_framecount);
        
    if (Global::shared_config.osd & SharedConfig::OSD_INPUTS)
        drawInputs(ai, preview_ai);

    if (Global::shared_config.osd & SharedConfig::OSD_MESSAGES)
        drawMessages();

    if (Global::shared_config.osd & SharedConfig::OSD_RAMWATCHES)
        drawWatches();

    if (Global::shared_config.osd & SharedConfig::OSD_LUA)
        drawLua();

    if (Global::shared_config.osd & SharedConfig::OSD_CROSSHAIR)
        drawCrosshair(ai);
}

}
