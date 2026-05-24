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

#ifndef LIBTAS_SCREENCAPTURE_IMPL_H_INCL
#define LIBTAS_SCREENCAPTURE_IMPL_H_INCL

#include <stdint.h>
#include <vector>

namespace libtas {

/**
 * @class ScreenCapture_Impl
 * @brief Abstract base class for backend-specific screen capture implementations.
 *
 * This class defines the interface used by the screen capture subsystem to
 * interact with platform-specific or API-specific capture backends.
 * Concrete subclasses implement the backend behavior for OpenGL, SDL, Vulkan,
 * XShm, VDPAU, and other capture methods.
 *
 * Virtual methods define the contract for initialization, capture, pixel
 * retrieval, restoration, and cleanup. Derived classes should document only
 * behavior that differs from the base contract.
 */
class ScreenCapture_Impl {

public:
    /**
     * @brief Destructor.
     */
    virtual ~ScreenCapture_Impl() {}
    
    /**
     * @brief Initializes backend-specific resources and detects display size.
     *
     * Sets up internal buffers and platform-specific state required for
     * screen capture.
     *
     * @return 0 on success, -1 on failure
     */
    virtual int init();

    /**
     * @brief Performs initialization steps after the backend is created.
     *
     * This helper is used internally after init() to finalize setup.
     *
     * @return 0 on success, -1 on failure
     */
    int postInit();

    /**
     * @brief Creates or allocates the capture surface or texture.
     *
     * This method is optional for backends that require an explicit render
     * target or capture surface.
     */
    virtual void initScreenSurface() {}

    /**
     * @brief Tears down backend-specific resources when capture is finished.
     */
    void fini();

    /**
     * @brief Destroys the capture surface or texture.
     *
     * Backends that allocate an intermediate surface should release it here.
     */
    virtual void destroyScreenSurface() {}

    /**
     * @brief Updates internal state after a screen resize.
     *
     * @param[in] w New width in pixels
     * @param[in] h New height in pixels
     */
    void resize(int w, int h);

    /**
     * @brief Retrieves current capture dimensions.
     *
     * @param[out] w Width in pixels
     * @param[out] h Height in pixels
     */
    void getDimensions(int& w, int& h);

    /**
     * @brief Returns the current pixel buffer size in bytes.
     *
     * @return Buffer size in bytes
     */
    int getSize();

    /**
     * @brief Returns the pixel format string used by the NUT muxer.
     *
     * Derived backends must provide the pixel format identifier expected by
     * the video encoder.
     *
     * @return Null-terminated pixel format string
     */
    virtual const char* getPixelFormat() = 0;

    /**
     * @brief Captures the current screen into the backend's internal surface.
     *
     * This may copy from the framebuffer, read pixels, or otherwise prepare
     * an intermediate capture target for later extraction.
     *
     * @return 0 on success, -1 on failure
     */
    virtual int copyScreenToSurface() = 0;

    /**
     * @brief Extracts pixels from the capture surface into a CPU buffer.
     *
     * The backend must provide a pointer to a pixel array through `pixels`.
     * The returned size indicates how many bytes are available.
     *
     * @param[out] pixels Returned pointer to pixel data
     * @param[in] draw Whether the capture should update the surface before readback
     *
     * @return Size of the pixel array in bytes
     */
    virtual int getPixelsFromSurface(uint8_t **pixels, bool draw) = 0;

    /**
     * @brief Restores the stored capture surface back to the screen.
     *
     * Used when capture operations modify the rendered output and the original
     * frame must be restored.
     *
     * @return 0 on success, -1 on failure
     */
    virtual int copySurfaceToScreen() = 0;

    /**
     * @brief Restores screen or backbuffer state after capture.
     *
     * By default this is a no-op. Backends may override if they need a
     * distinct restore step separate from copySurfaceToScreen().
     */
    virtual void restoreScreenState() {}

    /**
     * @brief Clears the current capture target.
     *
     * Optional override for backends that need to explicitly clear buffers or textures.
     */
    virtual void clearScreen() {}

    /**
     * @brief Returns an opaque capture target identifier.
     *
     * Useful for GPU-backed backends that render into a texture.
     *
     * @return Opaque texture or surface identifier, or 0 when unsupported
     */
    virtual uint64_t screenTexture() {return 0;}

protected:
    int copyPixelRows(const void* sourcePixels, int sourcePitch);

    
    /* Stored pixel array for use with the video encoder */
    std::vector<uint8_t> winpixels;

    /* Video dimensions */
    int width, height, pitch;
    unsigned int size;
    int pixelSize;

};
}

#endif
