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

#include "SurfaceARGB.h"
#include "../logging.h"

SurfaceARGB::SurfaceARGB(int width, int height)
{
    w = width;
    h = height;
    pitch = 4 * w;
    pixels.resize(w*h);
}

void SurfaceARGB::fill(uint32_t color)
{
    pixels.assign(w*h, color);
}

void SurfaceARGB::blit(SurfaceARGB* src, int x, int y)
{
    if ((x + src->w) > w) {
        debuglog(LCF_ERROR, "Cannot blit");
        return;
    }
    if ((y + src->h) > h) {
        debuglog(LCF_ERROR, "Cannot blit");
        return;
    }

    /* Code taken from SDL 2 software blitting BlitRGBtoRGBPixelAlpha() */
    int width = src->w;
    int height = src->h;
    uint32_t *srcp = src->pixels.data();
    uint32_t *dstp = pixels.data() + y*w + x;
    int dstskip = w - src->w;

    while (height--) {
        /* The code originally uses a Duff's device.
         * Let's try to trust the compiler optmisations.
         */
        while (width--) {
            uint32_t dalpha;
            uint32_t d;
            uint32_t s1;
            uint32_t d1;
            uint32_t s = *srcp;
            uint32_t alpha = s >> 24;
            /* FIXME: Here we special-case opaque alpha since the
               compositioning used (>>8 instead of /255) doesn't handle
               it correctly. Also special-case alpha=0 for speed?
               Benchmark this! */
            if (alpha) {
                if (alpha == 0xFF) {
                    *dstp = *srcp;
                } else {
                    /*
                     * take out the middle component (green), and process
                     * the other two in parallel. One multiply less.
                     */
                    d = *dstp;
                    dalpha = d >> 24;
                    s1 = s & 0xff00ff;
                    d1 = d & 0xff00ff;
                    d1 = (d1 + ((s1 - d1) * alpha >> 8)) & 0xff00ff;
                    s &= 0xff00;
                    d &= 0xff00;
                    d = (d + ((s - d) * alpha >> 8)) & 0xff00;
                    dalpha = alpha + (dalpha * (alpha ^ 0xFF) >> 8);
                    *dstp = d1 | d | (dalpha << 24);
                }
            }
            ++srcp;
            ++dstp;
        }
        dstp += dstskip;
    }
}


#endif

