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

#ifndef LIBTAS_IMGUI_LUADRAW_H_INCL
#define LIBTAS_IMGUI_LUADRAW_H_INCL

#include "../external/imgui/imgui.h"

#include <string>
#include <cstdint>

namespace libtas {

namespace LuaDraw
{

struct LuaShape
{
    virtual void render(ImDrawList* draw_list, ImVec2 offset, float scale) = 0;
    virtual bool isInbound() = 0;
};

struct LuaText : public LuaShape
{
    std::string text;
    uint32_t color;
    float x;
    float y;
    float anchor_x;
    float anchor_y;
    float font_size;
    bool monospace;
    static ImFont* regular_font;
    static ImFont* monospace_font;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override {return true;}
};

struct LuaWindow : public LuaShape
{
    std::string id;
    std::string text;
    float x;
    float y;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override {return true;}
};

struct LuaPixel : public LuaShape
{
    float x;
    float y;
    uint32_t color;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

struct LuaRect : public LuaShape
{
    float x;
    float y;
    float w;
    float h;
    float thickness;
    uint32_t color;
    int filled;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

struct LuaLine : public LuaShape
{
    float x0;
    float y0;
    float x1;
    float y1;
    uint32_t color;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

struct LuaQuad : public LuaShape
{
    float x0;
    float y0;
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float thickness;
    uint32_t color;
    int filled;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

struct LuaEllipse : public LuaShape
{
    float center_x;
    float center_y;
    float radius_x;
    float radius_y;
    float thickness;
    uint32_t color;
    int filled;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

/* Process incoming data from libTAS program */
void processSocket(int message);

/* Clear all lua drawings */
void reset();

void draw(ImDrawList* draw_list, ImVec2 offset, float scale);

/* Check if lua shape is inbound */
bool isInbound(float min_x, float min_y, float max_x, float max_y);

}

}

#endif
