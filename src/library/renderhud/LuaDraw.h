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
    virtual void render() = 0;
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
    void render() override;
};

struct LuaWindow : public LuaShape
{
    std::string id;
    std::string text;
    float x;
    float y;
    void render() override;
};

struct LuaPixel : public LuaShape
{
    float x;
    float y;
    uint32_t color;
    void render() override;
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
    void render() override;
};

struct LuaLine : public LuaShape
{
    float x0;
    float y0;
    float x1;
    float y1;
    uint32_t color;
    void render() override;
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
    void render() override;
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
    void render() override;
};

/* Insert a lua text to be displayed */
void insertText(float x, float y, std::string text, uint32_t color, float anchor_x, float anchor_y, float font_size, bool monospace);

/* Insert a lua window to be displayed */
void insertWindow(float x, float y, std::string id, std::string text);

/* Insert a lua pixel to be displayed */
void insertPixel(float x, float y, uint32_t color);

/* Insert a lua rect to be displayed */
void insertRect(float x, float y, float w, float h, float thickness, uint32_t color, int filled);

/* Insert a lua line to be displayed */
void insertLine(float x0, float y0, float x1, float y1, uint32_t color);

/* Insert a lua line to be displayed */
void insertQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness, uint32_t color, int filled);

/* Insert a lua line to be displayed */
void insertEllipse(float center_x, float center_y, float radius_x, float radius_y, float thickness, uint32_t color, int filled);

/* Clear all lua drawings */
void reset();

void draw();

/* Check if lua shape is inbound */
bool isInbound(float min_x, float min_y, float max_x, float max_y);

}

}

#endif
