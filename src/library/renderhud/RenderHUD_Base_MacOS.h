/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_RENDERHUD_BASE_MACOS_H_INCL
#define LIBTAS_RENDERHUD_BASE_MACOS_H_INCL

//#include "../../external/SDL.h"
#include <stdint.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>

namespace libtas {
/* This class provide a method to create a surface from
 * a text, using Core Text API. It returns a standard
 * 32-bit surface using the ARGB mask, encapsulated in a SurfaceARGB object.
 */
class RenderHUD_Base_MacOS : public RenderHUD
{
    public:
        ~RenderHUD_Base_MacOS();

        /* Initialize the font located at the given path */
        static void initFonts();

    private:
        /* Render text of specified color and outline
         * @param text      Text to display
         * @param fg_color  Color of the text
         * @param bg_color  Color of the outline of the text
         * @param x         x position of the text (top-left corner)
         * @param y         y position of the text (top-left corner)
         */
        void renderText(const char* text, Color fg_color, Color bg_color, int x, int y);

        static CTFontRef font;
        static CGFloat font_size;
};
}

#endif
