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

#ifdef __unix__
#include "config.h"
#endif

#ifndef LIBTAS_RENDERHUD_H_INCL
#define LIBTAS_RENDERHUD_H_INCL

#include "SurfaceARGB.h"

#include "TimeHolder.h"
#include "../shared/inputs/AllInputs.h"

#include <memory>
#include <list>
#include <utility>
#include <string>
#include <stdint.h>

namespace libtas {
/* This class handles the display of some text and shapes over the game screen (HUD).
 *
 * Because games have different methods of rendering, this class
 * should be derived for each rendering method. The subclass must
 * define the renderSurface() function
 *
 * Also, OS have different ways of creating a text surface from a string,
 * so this class must be derived for each OS to define the renderText() method.
 *
 * This class is also responsible on formatting and positioning the
 * different elements of the HUD.
 */
class RenderHUD
{
    public:
        virtual void newFrame() {}

        virtual void render() {}

        /* Display everything based on setting */
        void drawAll(uint64_t framecount, uint64_t nondraw_framecount, const AllInputs& ai, const AllInputs& preview_ai);

        /* Insert a message to be displayed */
        static void insertMessage(const char* message);

        /* Insert a ram watch to be displayed */
        static void insertWatch(std::string watch);

        /* Set marker text to be displayed */
        static void setMarkerText(std::string text);

        /* Clear the list of watches */
        static void resetWatches();

        /* Insert a lua text to be displayed */
        static void insertLuaText(int x, int y, std::string text, uint32_t color);

        /* Insert a lua pixel to be displayed */
        static void insertLuaPixel(int x, int y, uint32_t color);

        /* Insert a lua rect to be displayed */
        static void insertLuaRect(int x, int y, int w, int h, int thickness, uint32_t color, int filled);

        /* Insert a lua line to be displayed */
        static void insertLuaLine(int x0, int y0, int x1, int y1, uint32_t color);

        /* Insert a lua line to be displayed */
        static void insertLuaEllipse(int center_x, int center_y, int radius_x, int radius_y, uint32_t color);

        /* Clear all lua drawings */
        static void resetLua();

    private:
        /*** Draw specific information on screen ***/

        /* Display the frame count on screen */
        void drawFrame(uint64_t framecount, uint64_t nondraw_framecount);

        /* Display inputs and preview inputs on screen */
        void drawInputs(const AllInputs& ai, const AllInputs& preview_ai);

        /* Generic function to format inputs */
        std::string formatInputs(const AllInputs& ai);

        /* Display messages */
        void drawMessages();

        /* Display ram watches */
        void drawWatches();

        /* Display lua drawings */
        void drawLua();

        /* Display marker text */
        void drawMarkers();

        /* Display crosshair on current pointer position */
        void drawCrosshair(const AllInputs& ai);

        /* Messages to print on screen with the creation time */
        static std::list<std::pair<std::string, TimeHolder>> messages;

        /* Ram watches to print on screen */
        static std::list<std::string> watches;

        /* Marker text to print on screen */
        static std::string marker;

        struct LuaShape
        {
            virtual void render(RenderHUD *hud) = 0;
        };

        struct LuaText : public LuaShape
        {
            std::string text;
            Color color;
            int x;
            int y;
            void render(RenderHUD *hud) override;
        };

        struct LuaPixel : public LuaShape
        {
            int x;
            int y;
            Color color;
            void render(RenderHUD *hud) override;
        };

        struct LuaRect : public LuaShape
        {
            int x;
            int y;
            int w;
            int h;
            int thickness;
            Color color;
            int filled;
            void render(RenderHUD *hud) override;
        };

        struct LuaLine : public LuaShape
        {
            int x0;
            int y0;
            int x1;
            int y1;
            Color color;
            void render(RenderHUD *hud) override;
        };

        struct LuaEllipse : public LuaShape
        {
            int center_x;
            int center_y;
            int radius_x;
            int radius_y;
            Color color;
            void render(RenderHUD *hud) override;
        };

        /* Lua shapes to print on screen */
        static std::list<std::unique_ptr<LuaShape>> lua_shapes;

};
}

#endif
