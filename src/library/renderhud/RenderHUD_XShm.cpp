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
#include "../xlib/xshm.h" // x11::gameXImage

namespace libtas {

void RenderHUD_XShm::renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y)
{
    if (x11::gameXImage->bits_per_pixel == 32) {
        /* Create a SurfaceARGB pointing to the XImage */
        std::unique_ptr<SurfaceXImage> image_surf = std::unique_ptr<SurfaceXImage>(new SurfaceXImage(x11::gameXImage));

        image_surf->blit(surf.get(), x, y);
    }
    else {
        debuglogstdio(LCF_WINDOW | LCF_WARNING, "HUD for surface of depth %d is not supported", x11::gameXImage->bits_per_pixel);
    }
}

}

#endif
