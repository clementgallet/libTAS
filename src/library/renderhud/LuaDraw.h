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
    int x;
    int y;
    float anchor_x;
    float anchor_y;
    float font_size;
    bool monospace;
    static ImFont* regular_font;
    static ImFont* monospace_font;
    void render() override;
};

struct LuaPixel : public LuaShape
{
    int x;
    int y;
    uint32_t color;
    void render() override;
};

struct LuaRect : public LuaShape
{
    int x;
    int y;
    int w;
    int h;
    int thickness;
    uint32_t color;
    int filled;
    void render() override;
};

struct LuaLine : public LuaShape
{
    int x0;
    int y0;
    int x1;
    int y1;
    uint32_t color;
    void render() override;
};

struct LuaEllipse : public LuaShape
{
    int center_x;
    int center_y;
    int radius_x;
    int radius_y;
    uint32_t color;
    void render() override;
};

/* Insert a lua text to be displayed */
void insertText(int x, int y, std::string text, uint32_t color, float anchor_x, float anchor_y, float font_size, bool monospace);

/* Insert a lua pixel to be displayed */
void insertPixel(int x, int y, uint32_t color);

/* Insert a lua rect to be displayed */
void insertRect(int x, int y, int w, int h, int thickness, uint32_t color, int filled);

/* Insert a lua line to be displayed */
void insertLine(int x0, int y0, int x1, int y1, uint32_t color);

/* Insert a lua line to be displayed */
void insertEllipse(int center_x, int center_y, int radius_x, int radius_y, uint32_t color);

/* Clear all lua drawings */
void reset();

void draw();

}

}

#endif
