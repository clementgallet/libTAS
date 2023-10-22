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

#include "SurfaceARGB.h"

#include "../logging.h"

#include <cmath>

namespace libtas {

SurfaceARGB::SurfaceARGB(int width, int height)
{
    w = width;
    h = height;
    pitch = 4 * w;
    pixels.resize(w*h);
}

void SurfaceARGB::fill(Color color)
{
    pixels.assign(w*h, colorToValue(color));
}

void SurfaceARGB::fillBorder(Color color, int t)
{
    uint32_t value = colorToValue(color);
    
    for (int y=0; y<h; y++) {
        for (int x=(t<w)?(t-1):(w-1); x>=0; x--)
            pixels[y*w+x] = value;
        for (int x=(w-t)>=0?(w-t):0; x<w; x++)
            pixels[y*w+x] = value;
    }
    for (int x=0; x<w; x++) {
        for (int y=(t<h)?(t-1):(h-1); y>=0; y--)
            pixels[y*w+x] = value;
        for (int y=(h-t)>=0?(h-t):0; y<h; y++)
            pixels[y*w+x] = value;
    }
}

void SurfaceARGB::drawLine(Color color, bool positive_slope)
{ 
    /* Special case of line along one axis */
    if ((h == 1) || (w == 1)) {
        fill(color);
        return;
    }
    
    /* Pre-compute color values */ 
    uint32_t c = colorToValue(color);
    uint32_t orig_color = c & 0xffffff;
    uint32_t orig_alpha = color.a;
 
    /* Write end-line pixels */
    if (positive_slope) {
        pixels[0] = c;
        pixels[w*h-1] = c;
    }
    else {
        pixels[w-1] = c;
        pixels[w*(h-1)] = c;
    }
 
    const bool steep = h > w;
    const double gradient = steep ? ((double)(w-1)/(double)(h-1)) : ((double)(h-1)/(double)(w-1));
 
    double intery = gradient;
 
    if (steep) {
        for (int y = 1; y < (h-1); y++) {
            int x = std::floor(intery);
            uint32_t ratio = 256.0*(intery - (double)x);
            
            if (positive_slope) {
                pixels[y*w+x] = orig_color | (((orig_alpha * (256-ratio)) >> 8) << 24);
                pixels[y*w+x+1] = orig_color | (((orig_alpha * ratio) >> 8) << 24);
            }
            else {
                pixels[(h-1-y)*w+x] = orig_color | (((orig_alpha * (256-ratio)) >> 8) << 24);
                pixels[(h-1-y)*w+x+1] = orig_color | (((orig_alpha * ratio) >> 8) << 24);             
            }
            
            intery += gradient;
        }
    } else {
        for (int x = 1; x < (w-1); x++) {
            int y = std::floor(intery);
            uint32_t ratio = 256*(intery - y);
            
            if (positive_slope) {
                pixels[y*w+x] = orig_color | (((orig_alpha * (256-ratio)) >> 8) << 24);
                pixels[(y+1)*w+x] = orig_color | (((orig_alpha * ratio) >> 8) << 24);
            }
            else {
                pixels[y*w+(w-1-x)] = orig_color | (((orig_alpha * (256-ratio)) >> 8) << 24);
                pixels[(y+1)*w+(w-1-x)] = orig_color | (((orig_alpha * ratio) >> 8) << 24);
            }

            intery += gradient;
        }
    }
}

void SurfaceARGB::drawEllipse(Color color)
{
    /* Pre-compute color values */ 
    uint32_t c = colorToValue(color);
    uint32_t orig_color = c & 0xffffff;
    uint32_t orig_alpha = color.a;

    double radiusX = (w-1)/2;
    double radiusY = (h-1)/2;
    double radiusX2 = radiusX * radiusX;
    double radiusY2 = radiusY * radiusY;

    double quarter = std::round(radiusX2 / std::sqrt(radiusX2 + radiusY2));
    for (double dx = 0; dx <= quarter; dx++) {
        double dy = radiusY * std::sqrt(1 - dx * dx / radiusX2);
        uint32_t alpha = std::round(256*(dy - std::floor(dy)));

        setPixel4(radiusX, radiusY, dx, std::floor(dy), orig_color | (((orig_alpha * alpha) >> 8) << 24));
        setPixel4(radiusX, radiusY, dx, std::floor(dy) - 1, orig_color | (((orig_alpha * (256-alpha)) >> 8) << 24));
    }

    quarter = std::round(radiusY2 / std::sqrt(radiusX2 + radiusY2));
    for(double dy = 0; dy <= quarter; dy++) {
        double dx = radiusX * std::sqrt(1 - dy * dy / radiusY2);
        uint32_t alpha = std::round(256*(dx - std::floor(dx)));

        setPixel4(radiusX, radiusY, std::floor(dx), dy, orig_color | (((orig_alpha * alpha) >> 8) << 24));
        setPixel4(radiusX, radiusY, std::floor(dx) - 1, dy, orig_color | (((orig_alpha * (256-alpha)) >> 8) << 24));
    }
}

void SurfaceARGB::setPixel4(int x, int y, int dx, int dy, uint32_t c)
{
    pixels[(y+dy)*w+x+dx] = c;
    pixels[(y+dy)*w+x-dx] = c;
    pixels[(y-dy)*w+x+dx] = c;
    pixels[(y-dy)*w+x-dx] = c;
}
 
void SurfaceARGB::blit(const SurfaceARGB* src, int x, int y)
{
    /* Code taken from SDL 2 software blitting BlitRGBtoRGBPixelAlpha() */
    int width = src->w;
    int height = src->h;

    /* Check bounds */
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if ((x + width) > w)
        width = w - x;
    if ((y + height) > h)
        height = h - y;
    if (width <= 0)
        return;
    if (height <= 0)
        return;

    const uint32_t *srcp = src->pixels.data();
    uint32_t *dstp = pixels.data() + y*w + x;
    int dstskip = w - width;

    while (height--) {
        /* The code originally uses a Duff's device.
         * Let's try to trust the compiler optmisations.
         */
        int curwidth = width;
        while (curwidth--) {
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

uint32_t SurfaceARGB::colorToValue(Color color)
{
    uint32_t value = static_cast<uint32_t>(color.a);
    value = (value << 8) | static_cast<uint32_t>(color.r);
    value = (value << 8) | static_cast<uint32_t>(color.g);
    value = (value << 8) | static_cast<uint32_t>(color.b);
    return value;
}


}
