/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "RenderHUD_XShm.h"
#ifdef LIBTAS_ENABLE_HUD

#include "SurfaceARGB.h"
#include "SurfaceXImage.h"
#include "../logging.h"
#include "../hook.h"
#include "../ScreenCapture.h"

namespace libtas {

void RenderHUD_XShm::renderText(const char* text, Color fg_color, Color bg_color, int x, int y)
{
    std::unique_ptr<SurfaceARGB> surf = createTextSurface(text, fg_color, bg_color);

    if (!surf)
        return;

    if (gameXImage->bits_per_pixel == 32) {
        /* Create a SurfaceARGB pointing to the XImage */
        std::unique_ptr<SurfaceXImage> image_surf = std::unique_ptr<SurfaceXImage>(new SurfaceXImage(gameXImage));

        /* Change the coords so that the text fills on screen */
        int width, height;
        ScreenCapture::getDimensions(width, height);
        x = (x + surf->w + 5) > width ? (width - surf->w - 5) : x;
        y = (y + surf->h + 5) > height ? (height - surf->h - 5) : y;

        image_surf->blit(surf.get(), x, y);
    }
    else {
        debuglogstdio(LCF_WINDOW | LCF_WARNING, "HUD for surface of depth %d is not supported", gameXImage->bits_per_pixel);
    }
}

}

#endif
