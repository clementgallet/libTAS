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

#ifndef LIBTAS_SCREENCAPTURE_H_INCL
#define LIBTAS_SCREENCAPTURE_H_INCL

#include "ScreenCapture_Impl.h"

#include <stdint.h>
#include <vector>

namespace libtas {

/**
 * @class ScreenCapture
 * @brief Facade for the screen capture subsystem.
 *
 * This class provides a static interface to the active screen capture backend.
 * It delegates actual capture operations to a concrete ScreenCapture_Impl
 * implementation selected at runtime.
 *
 * The static API manages lifecycle, screen dimensions, and the capture flow:
 * initialization, screen copy, pixel extraction, and optional restoration of
 * the screen state.
 *
 * @note This interface is used internally by the screen capture subsystem and
 *       its driver implementations.
 */
class ScreenCapture {

public:
    /**
     * @brief Initializes the screen capture subsystem.
     *
     * Creates the selected backend implementation, allocates internal buffers,
     * and detects the current screen dimensions.
     *
     * @return 0 on success, -1 on error
     */
    static int init();

    /**
     * @brief Shuts down the screen capture subsystem.
     *
     * Cleans up allocated buffers and releases the backend implementation.
     */
    static void fini();

    /**
     * @brief Resizes the capture buffers to match the new screen dimensions.
     *
     * Called when the window or rendering target changes size.
     *
     * @param[in] w New width in pixels
     * @param[in] h New height in pixels
     */
    static void resize(int w, int h);

    /**
     * @brief Returns whether screen capture has been initialized.
     *
     * @return true if init() completed successfully and a backend is available
     */
    static bool isInited();

    /**
     * @brief Retrieves the current screen dimensions.
     *
     * @param[out] w Width in pixels
     * @param[out] h Height in pixels
     */
    static void getDimensions(int& w, int& h);

    /**
     * @brief Returns the number of bytes in the current pixel buffer.
     *
     * @return Size of the pixel array in bytes
     */
    static int getSize();

    /**
     * @brief Returns the pixel format identifier used by the video muxer.
     *
     * The returned string is the format name expected by the NUT muxer.
     *
     * @return Null-terminated pixel format string
     */
    static const char* getPixelFormat();

    /**
     * @brief Copies the current screen contents into the internal capture surface.
     *
     * The capture surface may be GPU-backed or platform-specific. This method
     * prepares the frame for later pixel extraction.
     *
     * @return Number of bytes captured on success, or -1 on failure
     */
    static int copyScreenToSurface();

    /**
     * @brief Extracts pixels from the capture surface into a CPU-accessible buffer.
     *
     * The pixel buffer is returned through the provided pointer and may be
     * reused until the next capture operation.
     *
     * @param[out] pixels Output pointer that receives the pixel buffer address
     * @param[in] draw Whether the surface should be drawn before extraction
     *
     * @return Size of the pixel buffer in bytes
     */
    static int getPixelsFromSurface(uint8_t **pixels, bool draw);

    /**
     * @brief Restores the stored capture surface back into the screen.
     *
     * This is used when the capture backend modifies the screen contents and
     * the original frame must be restored before the next render.
     *
     * @return 0 on success, or -1 on failure
     */
    static int copySurfaceToScreen();

    /**
     * @brief Restores the original screen state after capture.
     *
     * Equivalent to copySurfaceToScreen() in most backends, but may perform
     * additional restoration steps when needed.
     */
    static void restoreScreenState();

    /**
     * @brief Returns an opaque identifier for the current render target.
     *
     * Useful for backends that render into a texture or GPU surface.
     *
     * @return Opaque texture or surface identifier, or 0 when unsupported
     */
    static uint64_t screenTexture();

    /**
     * @brief Clears the current capture target.
     */
    static void clearScreen();

    /* width and height before screen capture has been implemented */
    static int width, height;

protected:
    
    static ScreenCapture_Impl* impl;
    static bool inited;
};
}

#endif
