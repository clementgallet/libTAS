/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_IMAGESCALINGSWS_H_INCL
#define LIBTAS_IMAGESCALINGSWS_H_INCL

#include "ImageScaling.h"

extern "C" {
#include <libswscale/swscale.h>
}

#include <vector>

namespace libtas {
/* Image scaling implementation using libswscale library */
class ImageScalingSws : public ImageScaling
{
public:
    ImageScalingSws();
    ~ImageScalingSws();

    bool isAvailable();

    bool isInited();

    void init(int src_width, int src_height, int src_pixfmt, int dst_width, int dst_height, int video_filter);

    const uint8_t* convertFrame(const uint8_t* src_data);

    void sourceHasResized(int width, int height);

private:
    /* Context for scaling images */
    struct SwsContext *sws_context;

    /* Version of the linked library */
    unsigned int sws_version;

    /* Scaling flags */
    int sws_flags = 0;

    /* Source image parameters */
    int src_width;
    int src_height;
    int src_depth;
    AVPixelFormat sws_format;
    int src_stride[4];
    int dst_stride[4];

    /* Image pixels */
    int dst_width;
    int dst_height;
    std::vector<uint8_t> dst_data;
    uint8_t* dst_planes[4];
};
}

#endif
