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

#include "LuaDraw.h"

#include "global.h" // Global::shared_config
#include "../external/imgui/imgui.h"
#include "TimeHolder.h"
#include "logging.h"
#include "screencapture/ScreenCapture.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <list>
#include <utility>
#include <string>
#include <memory>
#include <cmath>

namespace libtas {
    
static std::list<std::unique_ptr<LuaDraw::LuaShape>> lua_shapes;
ImFont* LuaDraw::LuaText::regular_font;
ImFont* LuaDraw::LuaText::monospace_font;
static int new_id = 0;

void LuaDraw::LuaText::render()
{
    ImFont* font = regular_font;
    
    if (monospace)
        font = monospace_font;
    
    /* Sanitize and process anchor values */
    if (anchor_x < 0.0f)
        anchor_x = 0.0f;
    if (anchor_x > 1.0f)
        anchor_x = 1.0f;
    if (anchor_y < 0.0f)
        anchor_y = 0.0f;
    if (anchor_y > 1.0f)
        anchor_y = 1.0f;
    
    /* Try avoiding computing the text length */
    if (anchor_x == 0.0f && anchor_y == 0.0f) {
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(x, y), color, text.c_str());
    }
    else {
        const ImVec2 size = font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, text.c_str(), NULL, NULL);
        float new_x = x - size.x * anchor_x;
        float new_y = y - size.y * anchor_y;
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(new_x, new_y), color, text.c_str());
    }    
}

void LuaDraw::LuaWindow::render()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoSavedSettings;
    if (id.empty())
        window_flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav;
    
    ImGui::SetNextWindowPos(ImVec2(x, y), id.empty() ? ImGuiCond_Always : ImGuiCond_Once, ImVec2(0.0f, 0.0f));

    /* Generate a unique id */
    std::string unique_id;
    if (id.empty()) {
        unique_id = "temp_";
        unique_id += std::to_string(new_id);
        new_id++;
    }

    if (ImGui::Begin(id.empty() ? unique_id.c_str() : id.c_str(), nullptr, window_flags)) {
        ImGui::TextUnformatted(text.c_str());
    }
    ImGui::End();
}

void LuaDraw::LuaPixel::render()
{
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y), color);
}

bool LuaDraw::LuaPixel::isInbound()
{
    return LuaDraw::isInbound(x, y, x, y);
}

void LuaDraw::LuaRect::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x+w, y+h), color);
    else
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x+w, y+h), color, 0.0f, 0, thickness);
}

bool LuaDraw::LuaRect::isInbound()
{
    return LuaDraw::isInbound(x, y, x+w, y+h);
}

void LuaDraw::LuaLine::render()
{
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color);
}

bool LuaDraw::LuaLine::isInbound()
{
    return LuaDraw::isInbound(std::min(x0, x1), std::min(y0, y1), std::max(x0, x1), std::max(y0, y1));
}

void LuaDraw::LuaQuad::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(x0, y0), ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color);
    else
        ImGui::GetBackgroundDrawList()->AddQuad(ImVec2(x0, y0), ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color, thickness);
}

bool LuaDraw::LuaQuad::isInbound()
{
    return LuaDraw::isInbound(std::min(std::min(x0, x1), std::min(x2, x3)),
                              std::min(std::min(y0, y1), std::min(y2, y3)),
                              std::max(std::max(x0, x1), std::max(x2, x3)),
                              std::max(std::max(y0, y1), std::max(y2, y3)));
}

void LuaDraw::LuaEllipse::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddEllipseFilled(ImVec2(center_x, center_y), radius_x, radius_y, color);
    else
        ImGui::GetBackgroundDrawList()->AddEllipse(ImVec2(center_x, center_y), radius_x, radius_y, color, 0.0f, 0, thickness);
}

bool LuaDraw::LuaEllipse::isInbound()
{
    return LuaDraw::isInbound(center_x - radius_x, center_y - radius_y, center_x + radius_x, center_y + radius_y);
}

void LuaDraw::processSocket(int message)
{
    switch (message) {
        case MSGN_LUA_RESOLUTION:
        {
            int w, h;
            ScreenCapture::getDimensions(w, h);
            sendMessage(MSGB_LUA_RESOLUTION);
            sendData(&w, sizeof(int));
            sendData(&h, sizeof(int));
            break;
        }
        case MSGN_LUA_TEXT:
        {
            auto lt = new LuaText();
            receiveData(&lt->x, sizeof(float));
            receiveData(&lt->y, sizeof(float));
            lt->text = receiveString();
            
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            lt->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                         static_cast<uint8_t>((color >> 8) & 0xff),
                         static_cast<uint8_t>(color & 0xff),
                         static_cast<uint8_t>((color >> 24) & 0xff));
            
            receiveData(&lt->anchor_x, sizeof(float));
            receiveData(&lt->anchor_y, sizeof(float));
            receiveData(&lt->font_size, sizeof(float));
            receiveData(&lt->monospace, sizeof(bool));
            lua_shapes.emplace_back(lt);
            break;
        }
        case MSGN_LUA_WINDOW:
        {
            auto lw = new LuaWindow();
            receiveData(&lw->x, sizeof(float));
            receiveData(&lw->y, sizeof(float));
            lw->id = receiveString();
            lw->text = receiveString();
            lua_shapes.emplace_back(lw);
            break;
        }
        case MSGN_LUA_PIXEL:
        {
            auto lp = std::unique_ptr<LuaPixel>(new LuaPixel);
            receiveData(&lp->x, sizeof(float));
            receiveData(&lp->y, sizeof(float));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            lp->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                         static_cast<uint8_t>((color >> 8) & 0xff),
                         static_cast<uint8_t>(color & 0xff),
                         static_cast<uint8_t>((color >> 24) & 0xff));

            if (lp->isInbound())
                lua_shapes.push_back(std::move(lp));
            break;
        }
        case MSGN_LUA_RECT:
        {
            auto lr = std::unique_ptr<LuaRect>(new LuaRect());
            receiveData(&lr->x, sizeof(float));
            receiveData(&lr->y, sizeof(float));
            receiveData(&lr->w, sizeof(float));
            receiveData(&lr->h, sizeof(float));
            receiveData(&lr->thickness, sizeof(float));

            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            lr->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                           static_cast<uint8_t>((color >> 8) & 0xff),
                           static_cast<uint8_t>(color & 0xff),
                           static_cast<uint8_t>((color >> 24) & 0xff));
            
            receiveData(&lr->filled, sizeof(int));

            if (lr->isInbound())
                lua_shapes.push_back(std::move(lr));
            break;
        }
        case MSGN_LUA_LINE:
        {
            auto ll = std::unique_ptr<LuaLine>(new LuaLine());
            receiveData(&ll->x0, sizeof(float));
            receiveData(&ll->y0, sizeof(float));
            receiveData(&ll->x1, sizeof(float));
            receiveData(&ll->y1, sizeof(float));

            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            ll->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                         static_cast<uint8_t>((color >> 8) & 0xff),
                         static_cast<uint8_t>(color & 0xff),
                         static_cast<uint8_t>((color >> 24) & 0xff));

            if (ll->isInbound())
                lua_shapes.push_back(std::move(ll));

            break;
        }
        case MSGN_LUA_QUAD:
        {
            auto lq = std::unique_ptr<LuaQuad>(new LuaQuad());
            receiveData(&lq->x0, sizeof(float));
            receiveData(&lq->y0, sizeof(float));
            receiveData(&lq->x1, sizeof(float));
            receiveData(&lq->y1, sizeof(float));
            receiveData(&lq->x2, sizeof(float));
            receiveData(&lq->y2, sizeof(float));
            receiveData(&lq->x3, sizeof(float));
            receiveData(&lq->y3, sizeof(float));
            receiveData(&lq->thickness, sizeof(float));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            lq->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                           static_cast<uint8_t>((color >> 8) & 0xff),
                           static_cast<uint8_t>(color & 0xff),
                           static_cast<uint8_t>((color >> 24) & 0xff));

            receiveData(&lq->filled, sizeof(int));
            
            if (lq->isInbound())
                lua_shapes.push_back(std::move(lq));    

            break;
        }
        case MSGN_LUA_ELLIPSE:
        {
            auto le = std::unique_ptr<LuaEllipse>(new LuaEllipse());
            receiveData(&le->center_x, sizeof(float));
            receiveData(&le->center_y, sizeof(float));
            receiveData(&le->radius_x, sizeof(float));
            receiveData(&le->radius_y, sizeof(float));
            receiveData(&le->thickness, sizeof(float));
            uint32_t color;
            receiveData(&color, sizeof(uint32_t));
            le->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                         static_cast<uint8_t>((color >> 8) & 0xff),
                         static_cast<uint8_t>(color & 0xff),
                         static_cast<uint8_t>((color >> 24) & 0xff));
            receiveData(&le->filled, sizeof(int));

            if (le->isInbound())
                lua_shapes.push_back(std::move(le));

            break;
        }
        default:
            break;
    }
}

void LuaDraw::draw()
{
    /* Reset the generation of unique ids */
    new_id = 0;
    
    for (auto &shape : lua_shapes) {
        shape->render();
    }
}

void LuaDraw::reset()
{
    lua_shapes.clear();
}

bool LuaDraw::isInbound(float min_x, float min_y, float max_x, float max_y)
{
    if (max_x < 0) return false;
    if (max_y < 0) return false;

    int w = 0, h = 0;
    ScreenCapture::getDimensions(w, h);

    if (min_x > w) return false;
    if (min_y > h) return false;

    return true;
}

}
