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

#include "RenderHUD_VDPAU.h"

#include "../logging.h"
#include "../hook.h"
#include "../../external/vdpau.h"

namespace libtas {

DECLARE_ORIG_POINTER(VdpBitmapSurfaceCreate)
DECLARE_ORIG_POINTER(VdpBitmapSurfaceDestroy)
DECLARE_ORIG_POINTER(VdpBitmapSurfacePutBitsNative)
DECLARE_ORIG_POINTER(VdpOutputSurfaceRenderBitmapSurface)

// RenderHUD_VDPAU::~RenderHUD_SDL2_renderer()
// {
// }

VdpDevice RenderHUD_VDPAU::device;

void RenderHUD_VDPAU::setDevice(VdpDevice d)
{
    device = d;
}

void RenderHUD_VDPAU::setSurface(VdpOutputSurface o)
{
    output_surface = o;
}

void RenderHUD_VDPAU::renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y)
{
    /* Create Vdp bitmap surface */
    VdpBitmapSurface surface;
    VdpStatus status = orig::VdpBitmapSurfaceCreate(device, VDP_RGBA_FORMAT_B8G8R8A8, surf->w, surf->h, false, &surface);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpBitmapSurfaceCreate failed with status %d", status);
        return;
    }

    /* Put surface pixels */
    uint32_t pitch = surf->pitch;
    void const* const pix = surf->pixels.data();
    status = orig::VdpBitmapSurfacePutBitsNative(surface, &pix, &pitch, nullptr);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpBitmapSurfacePutBitsNative failed with status %d", status);
        return;
    }

    VdpRect rect = {static_cast<uint32_t>(x), static_cast<uint32_t>(y), static_cast<uint32_t>(x + surf->w), static_cast<uint32_t>(y + surf->h)};

    /* Render the text on the output surface */
    VdpOutputSurfaceRenderBlendState blend_state;
    blend_state.struct_version = VDP_OUTPUT_SURFACE_RENDER_BLEND_STATE_VERSION;
    blend_state.blend_factor_source_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE;
    blend_state.blend_factor_destination_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_state.blend_factor_source_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE;
    blend_state.blend_factor_destination_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_state.blend_equation_color = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD;
    blend_state.blend_equation_alpha = VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD;
    status = orig::VdpOutputSurfaceRenderBitmapSurface(output_surface, &rect, surface, nullptr, nullptr, &blend_state, 0);
    if (status != VDP_STATUS_OK) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "VdpOutputSurfaceRenderBitmapSurface failed with status %d", status);
        return;
    }
}

}
