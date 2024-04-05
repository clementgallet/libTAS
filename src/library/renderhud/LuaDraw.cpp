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

#include <list>
#include <utility>
#include <string>
#include <memory>
#include <cmath>

namespace libtas {
    
static std::list<std::unique_ptr<LuaDraw::LuaShape>> lua_shapes;
ImFont* LuaDraw::LuaText::regular_font;
ImFont* LuaDraw::LuaText::monospace_font;

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

void LuaDraw::LuaPixel::render()
{
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y), color);
}

void LuaDraw::LuaRect::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x+w, y+h), color);
    else
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x+w, y+h), color, 0.0f, 0, thickness);
}

void LuaDraw::LuaLine::render()
{
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color);
}

void LuaDraw::LuaQuad::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddQuadFilled(ImVec2(x0, y0), ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color);
    else
        ImGui::GetBackgroundDrawList()->AddQuad(ImVec2(x0, y0), ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color, thickness);
}

void LuaDraw::LuaEllipse::render()
{
    if (filled)
        ImGui::GetBackgroundDrawList()->AddEllipseFilled(ImVec2(center_x, center_y), radius_x, radius_y, color);
    else
        ImGui::GetBackgroundDrawList()->AddEllipse(ImVec2(center_x, center_y), radius_x, radius_y, color, 0.0f, 0, thickness);
}

void LuaDraw::insertText(float x, float y, std::string text, uint32_t color, float anchor_x, float anchor_y, float font_size, bool monospace)
{
    auto lt = new LuaText();
    lt->x = x;
    lt->y = y;
    lt->text = text;
    lt->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
    lt->anchor_x = anchor_x;
    lt->anchor_y = anchor_y;
    lt->font_size = font_size;
    lt->monospace = monospace;
    lua_shapes.emplace_back(lt);
}

void LuaDraw::insertPixel(float x, float y, uint32_t color)
{
    auto lp = new LuaPixel;
    lp->x = x;
    lp->y = y;
    lp->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
    lua_shapes.emplace_back(lp);
}

void LuaDraw::insertRect(float x, float y, float w, float h, float thickness, uint32_t color, int filled)
{
    auto lr = new LuaRect();
    lr->x = x;
    lr->y = y;
    lr->w = w;
    lr->h = h;
    lr->thickness = thickness;
    lr->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                   static_cast<uint8_t>((color >> 8) & 0xff),
                   static_cast<uint8_t>(color & 0xff),
                   static_cast<uint8_t>((color >> 24) & 0xff));
    lr->filled = filled;
    lua_shapes.emplace_back(lr);
}

void LuaDraw::insertLine(float x0, float y0, float x1, float y1, uint32_t color)
{
    auto ll = new LuaLine();
    ll->x0 = x0;
    ll->y0 = y0;
    ll->x1 = x1;
    ll->y1 = y1;
    ll->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
    lua_shapes.emplace_back(ll);
}

void LuaDraw::insertQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness, uint32_t color, int filled)
{
    auto lq = new LuaQuad();
    lq->x0 = x0;
    lq->y0 = y0;
    lq->x1 = x1;
    lq->y1 = y1;
    lq->x2 = x2;
    lq->y2 = y2;
    lq->x3 = x3;
    lq->y3 = y3;
    lq->thickness = thickness;
    lq->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                   static_cast<uint8_t>((color >> 8) & 0xff),
                   static_cast<uint8_t>(color & 0xff),
                   static_cast<uint8_t>((color >> 24) & 0xff));
    lq->filled = filled;
    lua_shapes.emplace_back(lq);    
}

void LuaDraw::insertEllipse(float center_x, float center_y, float radius_x, float radius_y, float thickness, uint32_t color, int filled)
{
    auto le = new LuaEllipse();
    le->center_x = center_x;
    le->center_y = center_y;
    le->radius_x = radius_x;
    le->radius_y = radius_y;
    le->thickness = thickness;
    le->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
    le->filled = filled;
    lua_shapes.emplace_back(le);
}

void LuaDraw::draw()
{
    for (auto &shape : lua_shapes) {
        shape->render();
    }
}

void LuaDraw::reset()
{
    lua_shapes.clear();
}

}
