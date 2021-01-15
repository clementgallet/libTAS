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

#include "config.h"
#ifdef LIBTAS_ENABLE_HUD

#ifndef LIBTAS_SURFACEARGB_H_INCL
#define LIBTAS_SURFACEARGB_H_INCL

#include <cstdint>
#include <vector>

namespace libtas {
    
struct Color
{
    uint8_t r, g, b, a;
};

/* Create a simple ARGB surface class that can be used by sdl_ttf,
 * instead of the SDL_Surface struct.
 * This allows to use sdl_ttf without needing any function
 * from the SDL library.
 */
class SurfaceARGB
{
	public:
        /* Width and height */
        int w, h;

        /* Pitch */
        int pitch;

        /* Internal storage of surface pixels */
        std::vector<uint32_t> pixels;

        /* Pointer to the surface pixels (may not be pointing to internal storage)*/
        uint32_t* data;

        /* Allocate a surface with width and height*/
        SurfaceARGB(int width, int height);

        /* Fill all pixels to a certain color */
        void fill(Color color);

        /* Fill the border of the surface to a certain color and thickness */
        void fillBorder(Color color, int t);

        /* Blit surface `src` into this surface at coords x and y */
        void blit(const SurfaceARGB* src, int x, int y);
        
    private:
        /* Compute the value in ARGB32 from a color struct */
        uint32_t colorToValue(Color color);
};

}

#endif
#endif
