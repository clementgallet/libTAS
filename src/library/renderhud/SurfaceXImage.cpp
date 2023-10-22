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

#include "SurfaceXImage.h"

#include "../logging.h"

namespace libtas {

SurfaceXImage::SurfaceXImage(XImage* i)
{
    image = i;
}

void SurfaceXImage::blit(const SurfaceARGB* src, int x, int y)
{
    int dst_w = image->width;
    int dst_h = image->width;

    /* Code taken from SDL 2 software blitting BlitRGBtoRGBPixelAlpha() */
    int width = src->w;
    int height = src->h;

    /* Check bounds */
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if ((x + width) > dst_w)
        width = dst_w - x;
    if ((y + height) > dst_h)
        height = dst_h - y;
    if (width <= 0)
        return;
    if (height <= 0)
        return;

    const uint32_t *srcp = src->pixels.data();

    for (int ih = 0; ih < height; ih++) {
        /* The code originally uses a Duff's device.
         * Let's try to trust the compiler optmisations.
         */
        for (int iw = 0; iw < width; iw++) {
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
                    image->f.put_pixel(image, x + iw, y + ih, *srcp);
                } else {
                    /*
                     * take out the middle component (green), and process
                     * the other two in parallel. One multiply less.
                     */
                    d = image->f.get_pixel(image, x + iw, y + ih);
                    s1 = s & 0xff00ff;
                    d1 = d & 0xff00ff;
                    d1 = (d1 + ((s1 - d1) * alpha >> 8)) & 0xff00ff;
                    s &= 0xff00;
                    d &= 0xff00;
                    d = (d + ((s - d) * alpha >> 8)) & 0xff00;
                    image->f.put_pixel(image, x + iw, y + ih, d1 | d);
                }
            }
            ++srcp;
        }
    }
}

}
