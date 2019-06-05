/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "config.h"
#ifdef LIBTAS_ENABLE_HUD

#ifndef LIBTAS_RENDERHUD_H_INCL
#define LIBTAS_RENDERHUD_H_INCL

//#include "../../external/SDL.h"
#include "sdl_ttf.h"
#include "SurfaceARGB.h"
#include "../../shared/AllInputs.h"
#include "../TimeHolder.h"
#include <memory>
#include <list>
#include <utility>
#include <string>

namespace libtas {
/* This class handles the display of some text over the game screen (HUD).
 *
 * Because games have different methods of rendering, this class
 * should be derived for each rendering method. The subclass must
 * define the renderText() and size() functions
 *
 * As a helper, this class provide a method to create a surface from
 * a text, based on the sdl_ttf library. This library was modified so
 * that it does not depend on SDL anymore. It returns now a standard
 * 32-bit surface using the ARGB mask, encapsulated in a SurfaceARGB object.
 *
 * This class is also responsible on formatting and positioning the
 * different elements of the HUD.
 */
class RenderHUD
{
    public:
        RenderHUD();
        virtual ~RenderHUD();

        /* Initialize the font located at the given path */
        virtual void initFonts(const char* path);

        /* Main function to render some text on the screen.
         * This function does nothing in this class and must be overridden.
         * @param text      Text to display
         * @param fg_color  Color of the text
         * @param bg_color  Color of the outline of the text
         * @param x         x position of the text (top-left corner)
         * @param y         y position of the text (top-left corner)
         */
        virtual void renderText(const char* text, Color fg_color, Color bg_color, int x, int y) {};

        /* Display the frame count on screen */
        void renderFrame(uint64_t framecount);

        /* Display nondraw frame count on screen */
        void renderNonDrawFrame(uint64_t nondraw_framecount);

        /* Display the inputs on screen */
        void renderInputs(AllInputs& ai);

        /* Display the preview of inputs on screen */
        void renderPreviewInputs(AllInputs& ai);

        /* Display messages */
        void renderMessages();

        /* Insert a message to be displayed */
        static void insertMessage(const char* message);

        /* Display ram watches */
        void renderWatches();

        /* Insert a ram watch to be displayed */
        static void insertWatch(std::string watch);

        /* Clear the list of watches */
        static void resetWatches();

        /* Reset offsets to 0 */
        void resetOffsets();

    protected:
        /* Create a texture from a text, using colors for the text and the outline */
        std::unique_ptr<SurfaceARGB> createTextSurface(const char* text, Color fg_color, Color bg_color);

    private:
        /* Convert a location into screen coordinates, with an offset if text
         * was already displayed at that position.
         */
        void locationToCoords(int location, int& x, int& y);

        /* Generic function to display inputs on screen, using fg_color as the
         * text color */
        void renderInputs(AllInputs& ai, Color fg_color);

        int outline_size;
        int font_size;

        static TTF_Font* fg_font;
        static TTF_Font* bg_font;

        /* Location offsets when displaying multiple texts on the same location */
        int offsets[9];

        /* Messages to print on screen with the creation time */
        static std::list<std::pair<std::string, TimeHolder>> messages;

        /* Ram watches to print on screen */
        static std::list<std::string> watches;

};
}

#endif
#endif
