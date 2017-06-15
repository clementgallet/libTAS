/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef LIBTAS_ENABLE_HUD

#ifndef LIBTAS_SURFACEARGB_H_INCL
#define LIBTAS_SURFACEARGB_H_INCL

#include <cstdint>
#include <vector>

namespace libtas {
/* Create a simple ARGB surface class that can be used by sdl_ttf,
 * instead of the SDL_Surface struct.
 * This allows to use sdl_ttf without needing any function
 * from the SDL library.
 */
class SurfaceARGB
{
	public:
		int w, h;
		int pitch;
        std::vector<uint32_t> pixels;

        SurfaceARGB(int width, int height);

		void fill(uint32_t color);

        void blit(SurfaceARGB* src, int x, int y);
};

struct Color
{
    uint8_t r, g, b, a;
};

}

#endif
#endif
