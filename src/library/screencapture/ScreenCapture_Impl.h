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

#ifndef LIBTAS_SCREENCAPTURE_IMPL_H_INCL
#define LIBTAS_SCREENCAPTURE_IMPL_H_INCL

#include <stdint.h>
#include <vector>

namespace libtas {

class ScreenCapture_Impl {

public:
    virtual ~ScreenCapture_Impl() {}
    
    /* Initiate the internal variables and buffers, and get the screen dimensions
     * @return 0 if successful or -1 if an error occured
     */
    virtual int init();

    int postInit();

    /* Create the screen buffer/surface/texture */
    virtual void initScreenSurface() {}

    /* Called when screen is closed */
    void fini();

    /* Destroy the screen buffer/surface/texture */
    virtual void destroyScreenSurface() {}

    /* Called when the screen has been resized */
    void resize(int w, int h);

    /* Get the current dimensions of the screen */
    void getDimensions(int& w, int& h);

    /* Get the size of the pixel array */
    int getSize();

    /* Get the pixel format as an string used by nut muxer. */
    virtual const char* getPixelFormat() = 0;

    /* Copy the current screen to the screen buffer/surface/texture.
     * This surface is optimized for rendering, so it may be stored on GPU. */
    virtual int copyScreenToSurface() = 0;

    /* Transfer pixels from the screen buffer/surface/texture into an array, pointed by `pixels`
     * Returns the size of the array. */
    virtual int getPixelsFromSurface(uint8_t **pixels, bool draw) = 0;

    /* Copy back the stored screen buffer/surface/texture into the screen. */
    virtual int copySurfaceToScreen() = 0;

    /* Restore the state of the screen (backbuffer usually), because some methods
     * of rendering reuse the backbuffer from the previous frame.
     * It is equivalent to `copySurfaceToScreen()` in most cases. */
    virtual void restoreScreenState() {}

    virtual void clearScreen() {}

    /* Return an opaque identifier of the screen texture that was rendered to */
    virtual uint64_t screenTexture() {return 0;}

protected:
    
    /* Stored pixel array for use with the video encoder */
    std::vector<uint8_t> winpixels;

    /* Video dimensions */
    int width, height, pitch;
    unsigned int size;
    int pixelSize;

};
}

#endif
