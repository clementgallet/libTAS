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

#ifndef LIBTAS_IMAGESCALING_H_INCL
#define LIBTAS_IMAGESCALING_H_INCL

#include <cstdint>

namespace libtas {

/**
 * @class ImageScaling
 * @brief Abstract interface for image scaling.
 *
 * ImageScaling defines the interface for scaling images from one resolution
 * to another.
 *
 * Different backend implementations (e.g., libswscale) provide concrete
 * implementations of this interface. The specific implementation used depends
 * on the platform and available libraries.
 *
 * The scaling process involves:
 * 1. Initializing with scaling parameters
 * 2. Scaling input images to output images
 *
 * @see ImageScalingSws for libswscale implementation
 */
class ImageScaling
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ImageScaling() = default;

    /**
     * @brief Checks if the scaling backend is available on this platform.
     *
     * Returns whether the underlying scaling library/API is available and
     * can be used. Useful for checking if a specific implementation is available
     * before attempting to create or use an instance.
     *
     * @return true if the scaling is available, false otherwise
     *
     * @see isInited()
     */
    virtual bool isAvailable() = 0;

    /**
     * @brief Checks if the scaling is initialized with current parameters.
     *
     * Returns whether the scaling has been initialized with the current
     * parameters.
     *
     * @return true if initialized and ready to scale, false otherwise
     *
     * @see init()
     */
    virtual bool isInited() = 0;

    /**
     * @brief Initializes the scaling with input and output parameters.
     *
     * Configures the scaling to convert images from the specified input format
     * to the specified output format. The scaling state is reset during this
     * call. Subsequent calls to convertFrame() will use these parameters.
     *
     * @param[in] src_width  Width of the input image
     * @param[in] src_height Height of the input image
     * @param[in] src_pixfmt Pixel format of the input image
     * @param[in] dst_width  Width of the output image
     * @param[in] dst_height Height of the output image
     * @param[in] video_filter Video filter to apply during scaling
     */
    virtual void init(int src_width, int src_height, int src_pixfmt, int dst_width, int dst_height, int video_filter) = 0;

    /**
     * @brief Converts a single input image frame.
     *
     * Convert the pixels of a single input image frame from the input format to the output format.
     *
     * @param[in] src_data Pointer to input image data
     * @return pointer to the output pixels
     * @note Image data must be in the input format specified in init()
     *
     * @see init()
     */
    virtual const uint8_t* convertFrame(const uint8_t* src_data) = 0;

    /**
     * @brief Account for the input image changing resolution.
     *
     * Change the context parameters to account for the change in input image resolution
     *
     * @param[in] width  New width of the input image
     * @param[in] height New height of the input image
     */
    virtual void sourceHasResized(int width, int height) = 0;

};
} // namespace libtas

#endif
