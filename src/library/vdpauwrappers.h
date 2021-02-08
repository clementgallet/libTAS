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

#ifndef LIBTAS_VDPAU_H_INCL
#define LIBTAS_VDPAU_H_INCL

// #include <unistd.h>
#include "hook.h"
#include "global.h"
#include "../external/vdpau.h"
#include "../external/vdpau_x11.h"

namespace libtas {

namespace vdp {
    /* VDPAU device */
    extern VdpDevice vdpDevice;

    /* VDPAU output surface */
    extern VdpOutputSurface vdpSurface;
}

OVERRIDE int vdp_device_create_x11(Display *display, int screen, VdpDevice *device, VdpGetProcAddress **get_proc_address);

/* Functions used by RenderHUD_VDPAU */
VdpStatus VdpBitmapSurfaceCreate(VdpDevice device, VdpRGBAFormat rgba_format, uint32_t width, uint32_t height, VdpBool frequently_accessed, VdpBitmapSurface *surface);
VdpStatus VdpBitmapSurfaceDestroy(VdpBitmapSurface surface);
VdpStatus VdpBitmapSurfacePutBitsNative(VdpBitmapSurface surface, void const *const *source_data, uint32_t const *source_pitches, VdpRect const *destination_rect);
VdpStatus VdpOutputSurfaceRenderBitmapSurface(VdpOutputSurface destination_surface, VdpRect const *destination_rect, VdpBitmapSurface source_surface, VdpRect const *source_rect, VdpColor const *colors, VdpOutputSurfaceRenderBlendState const *blend_state, uint32_t flags);

VdpStatus VdpOutputSurfaceGetParameters(VdpOutputSurface surface, VdpRGBAFormat *rgba_format, uint32_t *width, uint32_t *height);
VdpStatus VdpOutputSurfaceCreate(VdpDevice device, VdpRGBAFormat rgba_format, uint32_t width, uint32_t height, VdpOutputSurface *surface);
VdpStatus VdpOutputSurfaceDestroy(VdpOutputSurface surface);
VdpStatus VdpOutputSurfaceRenderOutputSurface(VdpOutputSurface destination_surface, VdpRect const *destination_rect, VdpOutputSurface source_surface, VdpRect const *source_rect, VdpColor const *colors, VdpOutputSurfaceRenderBlendState const *blend_state, uint32_t flags);
VdpStatus VdpOutputSurfaceGetBitsNative(VdpOutputSurface surface, VdpRect const *source_rect, void *const *destination_data, uint32_t const *destination_pitches);

}

#endif
