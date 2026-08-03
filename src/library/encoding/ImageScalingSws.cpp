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

#include "ImageScalingSws.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"
#include "../shared/SharedConfig.h"
#include "screencapture/ScreenCapture.h"

extern "C" {
#include <libavutil/samplefmt.h>
}
#include <stdlib.h>
#include <stdint.h>

namespace libtas {

/* Link dynamically to swscale functions, because there are different
 * library versions depending on your distro.
 */
DEFINE_ORIG_POINTER(swscale_version)
DEFINE_ORIG_POINTER(sws_scale_frame)
DEFINE_ORIG_POINTER(sws_getContext)
DEFINE_ORIG_POINTER(sws_scale)
DEFINE_ORIG_POINTER(sws_freeContext)

ImageScalingSws::ImageScalingSws(void)
{
    /* Some systems don't create the unversionned symlinks when the libraries
     * are installed, so we add a link with the major version. */

    /* Disabling logging because we expect some of these to fail */
    {
        GlobalNoLog gnl;
        LINK_NAMESPACE(sws_getContext, "swscale");
        LINK_NAMESPACE_FULLNAME(sws_getContext, "libswscale.so.9");
        LINK_NAMESPACE_FULLNAME(sws_getContext, "libswscale.so.8");
        LINK_NAMESPACE_FULLNAME(sws_getContext, "libswscale.so.7");
        LINK_NAMESPACE_FULLNAME(sws_getContext, "libswscale.so.6");
    }
    /* Still test if it succeeded. */
    if (!orig::sws_getContext) {
        LOG(LL_WARN, LCF_DUMP, "Could not link to sws_getContext, video scaling will not be available");
        sws_context = nullptr;
    }
    else {
        LINK_NAMESPACE(swscale_version, "swscale");
        sws_version = orig::swscale_version();

        /* We link to the other functions here, because linking during destructor can softlock */
        LINK_NAMESPACE(sws_scale, "swscale");
        LINK_NAMESPACE(sws_freeContext, "swscale");
    }
}

ImageScalingSws::~ImageScalingSws(void)
{
    if (orig::sws_freeContext && sws_context)
        orig::sws_freeContext(sws_context);
}

bool ImageScalingSws::isAvailable()
{
    return orig::sws_getContext;
}

bool ImageScalingSws::isInited()
{
    return sws_context;
}

void ImageScalingSws::init(int src_width, int src_height, int src_pixfmt, int dst_width, int dst_height, int video_filter)
{
    sws_format = AV_PIX_FMT_NONE;
    switch(src_pixfmt) {
        case ScreenCapture::PIXELFORMAT_RGBA8:
            sws_format = AV_PIX_FMT_RGBA;
            break;
        case ScreenCapture::PIXELFORMAT_BGRA8:
            sws_format = AV_PIX_FMT_BGRA;
            break;
        case ScreenCapture::PIXELFORMAT_ARGB8:
            sws_format = AV_PIX_FMT_ARGB;
            break;
        case ScreenCapture::PIXELFORMAT_ABGR8:
            sws_format = AV_PIX_FMT_ABGR;
            break;
        case ScreenCapture::PIXELFORMAT_RGBX8:
            sws_format = AV_PIX_FMT_RGB0;
            break;
        case ScreenCapture::PIXELFORMAT_BGRX8:
            sws_format = AV_PIX_FMT_BGR0;
            break;
        case ScreenCapture::PIXELFORMAT_XRGB8:
            sws_format = AV_PIX_FMT_0RGB;
            break;
        case ScreenCapture::PIXELFORMAT_XBGR8:
            sws_format = AV_PIX_FMT_0BGR;
            break;
        case ScreenCapture::PIXELFORMAT_RGB24:
            sws_format = AV_PIX_FMT_RGB24;
            break;
        case ScreenCapture::PIXELFORMAT_BGR24:
            sws_format = AV_PIX_FMT_BGR24;
            break;
        case ScreenCapture::PIXELFORMAT_RGBA16:
            sws_format = AV_PIX_FMT_BGR48LE;
            break;
        default:
            LOG(LL_ERROR, LCF_DUMP, "Unsupported pixel format for scaling");
            return;
    }

    sws_flags = 0;
    switch(video_filter) {
        case SharedConfig::VFILTER_POINT:
            sws_flags = SWS_POINT;
            break;
        case SharedConfig::VFILTER_BILINEAR:
            sws_flags = SWS_BILINEAR;
            break;
        case SharedConfig::VFILTER_BICUBIC:
            sws_flags = SWS_BICUBIC;
            break;
    }

    /* Open the context */
    sws_context = orig::sws_getContext(src_width, src_height, sws_format, dst_width, dst_height, sws_format, sws_flags, nullptr, nullptr, nullptr);

    if (!sws_context) {
        LOG(LL_ERROR, LCF_DUMP, "Error initializing sws context");
        return;
    }

    /* Save source parameters */
    this->src_width = src_width;
    this->src_height = src_height;
    src_depth = ScreenCapture::getPixelFormatDepth(src_pixfmt);
    src_stride[0] = src_width * src_depth;

    /* Prepare the scaled pixel data */
    this->dst_width = dst_width;
    this->dst_height = dst_height;
    dst_data.resize(dst_width * dst_height * src_depth);
    dst_planes[0] = dst_data.data();
    dst_stride[0] = dst_width * src_depth;
}

const uint8_t* ImageScalingSws::convertFrame(const uint8_t* src_data)
{
    if (!sws_context)
        return nullptr;

    /* Convert to destination format */
    orig::sws_scale(sws_context, (const uint8_t * const*)&src_data, src_stride, 0, src_height, dst_planes, dst_stride);
    return dst_planes[0];
}

void ImageScalingSws::sourceHasResized(int width, int height)
{
    if (!sws_context)
        return;

    src_width = width;
    src_height = height;
    src_stride[0] = src_width * src_depth;

    orig::sws_freeContext(sws_context);

    sws_context = orig::sws_getContext(src_width, src_height, sws_format, dst_width, dst_height, sws_format, sws_flags, nullptr, nullptr, nullptr);

    if (!sws_context) {
        LOG(LL_ERROR, LCF_DUMP, "Error initializing sws context");
        return;
    }
}

} // namespace libtas
