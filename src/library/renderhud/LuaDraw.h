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

#ifndef LIBTAS_IMGUI_LUADRAW_H_INCL
#define LIBTAS_IMGUI_LUADRAW_H_INCL

#include "../external/imgui/imgui.h"

#include <string>
#include <cstdint>

namespace libtas {

/**
 * @namespace LuaDraw
 * @brief Helper namespace for rendering Lua-driven overlay primitives.
 *
 * Provides shape abstractions and rendering helpers for Lua-generated HUD
 * drawing commands.
 */
namespace LuaDraw
{

struct LuaShape
{
    virtual void render(ImDrawList* draw_list, ImVec2 offset, float scale) = 0;
    virtual bool isInbound() = 0;
};

/**
 * @struct LuaText
 * @brief Represents a text shape drawn by Lua scripts.
 */
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

/**
 * @struct LuaWindow
 * @brief Represents a Lua-drawn window overlay.
 */
struct LuaWindow : public LuaShape
{
    std::string id;
    std::string text;
    float x;
    float y;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override {return true;}
};

/**
 * @struct LuaPixel
 * @brief Represents a single pixel drawn by Lua scripts.
 */
struct LuaPixel : public LuaShape
{
    float x;
    float y;
    uint32_t color;
    void render(ImDrawList* draw_list, ImVec2 offset, float scale) override;
    bool isInbound() override;
};

/**
 * @struct LuaRect
 * @brief Represents a rectangle shape drawn by Lua scripts.
 */
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

/**
 * @struct LuaLine
 * @brief Represents a line shape drawn by Lua scripts.
 */
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

/**
 * @struct LuaQuad
 * @brief Represents a quadrilateral shape drawn by Lua scripts.
 */
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

/**
 * @struct LuaEllipse
 * @brief Represents an ellipse shape drawn by Lua scripts.
 */
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

/**
 * @brief Processes an incoming Lua drawing message.
 *
 * @param[in] message Message identifier received from libTAS
 */
void processSocket(int message);

/**
 * @brief Clears all Lua drawing primitives.
 */
void reset();

/**
 * @brief Renders the current Lua drawing buffer.
 *
 * @param[in] draw_list ImGui draw list to render into
 * @param[in] offset Offset applied to drawn shapes
 * @param[in] scale Scale applied to drawn shapes
 */
void draw(ImDrawList* draw_list, ImVec2 offset, float scale);

/**
 * @brief Tests whether a Lua shape lies within a given bounding box.
 *
 * @param[in] min_x Minimum x coordinate
 * @param[in] min_y Minimum y coordinate
 * @param[in] max_x Maximum x coordinate
 * @param[in] max_y Maximum y coordinate
 * @return true if the shape is inbound to the box
 */
bool isInbound(float min_x, float min_y, float max_x, float max_y);

}

}

#endif
