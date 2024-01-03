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

#include <list>
#include <utility>
#include <string>
#include <memory>

namespace libtas {
    
static std::list<std::unique_ptr<LuaDraw::LuaShape>> lua_shapes;

void LuaDraw::LuaText::render()
{
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), color, text.c_str());
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

void LuaDraw::LuaEllipse::render()
{
    ImGui::GetBackgroundDrawList()->AddEllipse(ImVec2(center_x, center_y), radius_x, radius_y, color);
}

void LuaDraw::insertText(int x, int y, std::string text, uint32_t color)
{
    auto lt = new LuaText();
    lt->x = x;
    lt->y = y;
    lt->text = text;
    lt->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
    lua_shapes.emplace_back(lt);
}

void LuaDraw::insertPixel(int x, int y, uint32_t color)
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

void LuaDraw::insertRect(int x, int y, int w, int h, int thickness, uint32_t color, int filled)
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

void LuaDraw::insertLine(int x0, int y0, int x1, int y1, uint32_t color)
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

void LuaDraw::insertEllipse(int center_x, int center_y, int radius_x, int radius_y, uint32_t color)
{
    auto le = new LuaEllipse();
    le->center_x = center_x;
    le->center_y = center_y;
    le->radius_x = radius_x;
    le->radius_y = radius_y;
    le->color = IM_COL32(static_cast<uint8_t>((color >> 16) & 0xff),
                 static_cast<uint8_t>((color >> 8) & 0xff),
                 static_cast<uint8_t>(color & 0xff),
                 static_cast<uint8_t>((color >> 24) & 0xff));
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
