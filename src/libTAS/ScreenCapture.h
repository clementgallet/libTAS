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

#ifndef LIBTAS_SCREENCAPTURE_H_INCL
#define LIBTAS_SCREENCAPTURE_H_INCL

#include <stdint.h>
#include <SDL2/SDL.h>
#include <vector>

namespace libtas {

namespace ScreenCapture {

/* Initiate the internal variables and buffers, and get the screen dimensions
 * @return 0 if successful or -1 if an error occured
 */
int init(SDL_Window* window);

/* Called when screen is closed */
void fini();

/* Called when screen is resized */
void reinit(SDL_Window* window);

bool isInited();

void getDimensions(int& w, int& h);


/* Get the pixel format as an string used by nut muxer. */
const char* getPixelFormat();


/* Capture the pixels from the screen. */
int storePixels();

/* Capture the pixels from the screen and copy a pointer to this array into pixels.
 * Returns the size of the array.
 */
int getPixels(uint8_t **pixels, bool draw);

/* Set the screen pixels from our buffers.
 * @param same  if true, the screen did not change since last call with same==true
 */
int setPixels(bool same);

}
}

#endif
