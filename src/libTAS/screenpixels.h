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

#ifndef LIBTAS_SCREENPIXELS_H_INCL
#define LIBTAS_SCREENPIXELS_H_INCL

#include <stdint.h>
#include <SDL2/SDL.h>
#ifdef LIBTAS_ENABLE_AVDUMPING
extern "C" {
#include <libavutil/pixfmt.h>
}
#endif

namespace libtas {

/* Initiate the internal variables and buffers, and get the screen dimensions
 * @return 0 if successful or -1 if an error occured
 */
int initScreenPixels(SDL_Window* window, bool video_opengl, int *pwidth, int *pheight);

/* Called when screen is closed or resized */
void finiScreenPixels();

#ifdef LIBTAS_ENABLE_AVDUMPING

/* Get the pixel format as an enum used by ffmpeg library. */
AVPixelFormat getPixelFormat(SDL_Window* window);

#endif

/* Capture the pixels from the screen and copy it to the following structs:
 * @param plane   Array of 4 elements containing a pointer to list of
 *                pixel values for each plane. For non-planar formats
 *                (like RGB/RGBA), all pixels are stored in the first list
 * @param stride  Array of 4 elements containing the size in bytes of a
 *                row of pixels for each plane. For non-planar formats,
 *                the first element contains width * (size of a pixel).
 * @return        0 if successful or -1 if an error occured
 */
int getScreenPixels(const uint8_t* orig_plane[], int orig_stride[]);

/* Set the screen pixels from our buffers */
int setScreenPixels(SDL_Window* window);

}

#endif
