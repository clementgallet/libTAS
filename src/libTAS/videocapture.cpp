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

#include "videocapture.h"

#ifndef LIBTAS_DISABLE_AVDUMPING

#include "hook.h"
#include "logging.h"
#include "../external/gl.h" // glReadPixels enum arguments
#include "../external/SDL.h" // SDL_Surface
#include <vector>
#include <string.h> // memcpy


int useGL;

/* Temporary pixel arrays */
std::vector<uint8_t> glpixels;
std::vector<uint8_t> winpixels;

/* Original function pointers */
void (*SDL_GL_GetDrawableSize_real)(void* window, int* w, int* h);
SDL_Surface* (*SDL_GetWindowSurface_real)(void* window);
int (*SDL_LockSurface_real)(SDL_Surface* surface);
void (*SDL_UnlockSurface_real)(SDL_Surface* surface);
void (*glReadPixels_real)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* data);

/* Video dimensions */
int width, height;
int size;

int initVideoCapture(void* window, int video_opengl, int *pwidth, int *pheight)
{
    /* FIXME: Does not work with SDL 1.2 !!! */
    LINK_SUFFIX(SDL_GL_GetDrawableSize, "libSDL2-2");
    LINK_SUFFIX(SDL_GetWindowSurface, "libSDL2-2");
    LINK_SUFFIX(SDL_LockSurface, "libSDL2-2");
    LINK_SUFFIX(SDL_UnlockSurface, "libSDL2-2");
    LINK_SUFFIX(glReadPixels, "libGL");

    /* Get information about the current screen */
    SDL_GL_GetDrawableSize_real(window, pwidth, pheight);

    /* Save dimensions for later */
    width = *pwidth;
    height = *pheight;

    /* Allocate an array of pixels */
    size = width * height * 4;
    winpixels.resize(size);

    /* Dimensions must be a multiple of 2 */
    if ((width % 1) || (height % 1)) {
        debuglog(LCF_DUMP | LCF_ERROR, "Screen dimensions must be a multiple of 2");
        return 1;
    }

    /* If the game uses openGL, the method to capture the screen will be different */
    useGL = video_opengl;

    if (useGL) {
        /* Do we already have access to the glReadPixels function? */
        if (!glReadPixels_real) {
            debuglog(LCF_DUMP | LCF_OGL | LCF_ERROR, "Could not load function glReadPixels.");
            return 1;
        }

        /* Allocate another pixels array,
         * because the image will need to be flipped.
         */
        glpixels.resize(size);
    }
}

int captureVideoFrame(void *window, const uint8_t* orig_plane[], int orig_stride[])
{
    SDL_Surface* surface = NULL;

    if (useGL) {
        /* TODO: Check that the openGL dimensions did not change in between */

        /* We access to the image pixels directly using glReadPixels */
        glReadPixels_real(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &glpixels[0]);
        /* TODO: I saw this in some examples before calling glReadPixels: glPixelStorei(GL_PACK_ALIGNMENT, 1); */

        /*
         * Flip image horizontally
         * This is because OpenGL has a different reference point
         * Code taken from http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black
         * TODO: Could this be done without allocating another array ?
         */

        int pitch = 4 * width;
        for (int line = 0; line < height; line++) {
            int pos = line * pitch;
            for (int p=0; p<pitch; p++) {
                winpixels[pos + p] = glpixels[(size-pos)-pitch + p];
            }
        }
    }

    else {
        /* Not tested !! */
        debuglog(LCF_DUMP | LCF_UNTESTED | LCF_FRAME, "Access SDL_Surface pixels for video dump");

        /* Get surface from window */
        surface = SDL_GetWindowSurface_real(window);

        /* Currently only supporting ARGB (32 bpp) pixel format */
        if (surface->format->BitsPerPixel != 32) {
            debuglog(LCF_DUMP | LCF_ERROR, "Bad bpp for surface: %d\n", surface->format->BitsPerPixel);
            return 1;
        }

        /* Checking for a size modification */
        if ((surface->w != width) || (surface->h != height)) {
            debuglog(LCF_DUMP | LCF_ERROR, "Window coords have changed (",width,",",height,") -> (",surface->w,",",surface->h,")");
            return 1;
        }

        /* We must lock the surface before accessing the raw pixels */
        if (0 != SDL_LockSurface_real(surface)) {
            debuglog(LCF_DUMP | LCF_ERROR, "Could not lock SDL surface");
            return 1;
        }

        /* I know memcpy is not recommended for vectors... */
        memcpy(&winpixels[0], surface->pixels, size);

        /* Unlock surface */
        SDL_UnlockSurface_real(surface);
    }

    orig_plane[0] = (const uint8_t*)(&winpixels[0]);
    orig_stride[0] = width * 4;

}

#endif

