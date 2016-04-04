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

#ifndef LIBTAS_VIDEOCAPTURE_H_INCL
#define LIBTAS_VIDEOCAPTURE_H_INCL

#ifdef LIBTAS_ENABLE_AVDUMPING

#include <stdint.h>

/* Initiate the video capture and get the screen dimensions
 * @return 0 if successful or 1 if an error occured
 */
int initVideoCapture(void* window, int video_opengl, int *pwidth, int *pheight);

/* Capture the pixels from the screen and copy it to the following structs:
 * @param plane   Array of 4 elements containing a pointer to list of
 *                pixel values for each plane. For non-planar formats
 *                (like RGB/RGBA), all pixels are stored in the first list
 * @param stride  Array of 4 elements containing the size in bytes of a
 *                row of pixels for each plane. For non-planar formats,
 *                the first element contains width * (size of a pixel).
 * @return        0 if successful or 1 if an error occured
 */
int captureVideoFrame(void *window, const uint8_t* orig_plane[], int orig_stride[]);

#endif
#endif

