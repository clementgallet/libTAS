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

#include "vdpauwrappers.h"
#include "logging.h"
#include "GlobalState.h"
#include "ScreenCapture.h"
#include "frame.h"
#include "global.h"
#include "renderhud/RenderHUD_VDPAU.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "xlib/xwindows.h" // x11::gameXWindows
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

namespace libtas {

VdpDevice vdp::vdpDevice = 0;
VdpOutputSurface vdp::vdpSurface = 0;

VdpStatus VdpPresentationQueueTargetCreateX11(VdpDevice device, Drawable drawable, VdpPresentationQueueTarget *target);
VdpStatus VdpPresentationQueueCreate(VdpDevice device, VdpPresentationQueueTarget presentation_queue_target, VdpPresentationQueue *presentation_queue);
VdpStatus VdpPresentationQueueDestroy(VdpPresentationQueue presentation_queue);
VdpStatus VdpPresentationQueueDisplay(VdpPresentationQueue presentation_queue, VdpOutputSurface surface, uint32_t clip_width, uint32_t clip_height, VdpTime earliest_presentation_time);
VdpStatus VdpPresentationQueueBlockUntilSurfaceIdle(VdpPresentationQueue presentation_queue, VdpOutputSurface surface, VdpTime *first_presentation_time);

DEFINE_ORIG_POINTER(vdp_device_create_x11)
DEFINE_ORIG_POINTER(VdpPresentationQueueTargetCreateX11)
DEFINE_ORIG_POINTER(VdpPresentationQueueCreate)
DEFINE_ORIG_POINTER(VdpPresentationQueueDestroy)
DEFINE_ORIG_POINTER(VdpPresentationQueueDisplay)
DEFINE_ORIG_POINTER(VdpPresentationQueueBlockUntilSurfaceIdle)

DEFINE_ORIG_POINTER(VdpBitmapSurfaceCreate)
DEFINE_ORIG_POINTER(VdpBitmapSurfaceDestroy)
DEFINE_ORIG_POINTER(VdpBitmapSurfacePutBitsNative)
DEFINE_ORIG_POINTER(VdpOutputSurfaceRenderBitmapSurface)

DEFINE_ORIG_POINTER(VdpOutputSurfaceGetParameters)
DEFINE_ORIG_POINTER(VdpOutputSurfaceCreate)
DEFINE_ORIG_POINTER(VdpOutputSurfaceDestroy)
DEFINE_ORIG_POINTER(VdpOutputSurfaceRenderOutputSurface)
DEFINE_ORIG_POINTER(VdpOutputSurfaceGetBitsNative)

namespace orig {
    VdpGetProcAddress* GetProcAddress = nullptr;
}

VdpStatus VdpPresentationQueueTargetCreateX11(VdpDevice device, Drawable drawable, VdpPresentationQueueTarget *target)
{
    if (GlobalState::isNative())
        return orig::VdpPresentationQueueTargetCreateX11(device, drawable, target);

    DEBUGLOGCALL(LCF_WINDOW);

    /* Set the game window to that window */

    /* Remove window from the list if it is already present */
    for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
        if (drawable == *iter) {
            x11::gameXWindows.erase(iter);
            break;
        }
    }

    x11::gameXWindows.push_front(drawable);
    uint32_t i = static_cast<uint32_t>(drawable);
    lockSocket();
    sendMessage(MSGB_WINDOW_ID);
    sendData(&i, sizeof(i));
    unlockSocket();
    debuglogstdio(LCF_WINDOW, "Sent X11 window id %d", i);

    return orig::VdpPresentationQueueTargetCreateX11(device, drawable, target);
}

VdpStatus VdpPresentationQueueCreate(VdpDevice device, VdpPresentationQueueTarget presentation_queue_target, VdpPresentationQueue *presentation_queue)
{
    if (GlobalState::isNative())
        return orig::VdpPresentationQueueCreate(device, presentation_queue_target, presentation_queue);

    DEBUGLOGCALL(LCF_WINDOW);

    ScreenCapture::fini();

    vdp::vdpDevice = device;

    Global::game_info.video |= GameInfo::VDPAU;
    Global::game_info.tosend = true;

    RenderHUD_VDPAU::setDevice(device);

    return orig::VdpPresentationQueueCreate(device, presentation_queue_target, presentation_queue);
}

VdpStatus VdpPresentationQueueDestroy(VdpPresentationQueue presentation_queue)
{
    if (GlobalState::isNative())
        return orig::VdpPresentationQueueDestroy(presentation_queue);

    DEBUGLOGCALL(LCF_WINDOW);

    ScreenCapture::fini();

    return orig::VdpPresentationQueueDestroy(presentation_queue);
}

VdpStatus VdpPresentationQueueDisplay(VdpPresentationQueue presentation_queue, VdpOutputSurface surface, uint32_t clip_width, uint32_t clip_height, VdpTime earliest_presentation_time)
{
    if (GlobalState::isNative())
        return orig::VdpPresentationQueueDisplay(presentation_queue, surface, clip_width, clip_height, earliest_presentation_time);

    debuglogstdio(LCF_WINDOW, "%s called with clip_width %d, clip_height %d and earliest_presentation_time %llu", __func__, clip_width, clip_height, earliest_presentation_time);

    vdp::vdpSurface = surface;

    /* The surface can change size independently of the window size, so we
     * must check here everytime. */
    VdpRGBAFormat rgba_format;
    unsigned int uw, uh;
    orig::VdpOutputSurfaceGetParameters(surface, &rgba_format, &uw, &uh);

    /* Resize the screen capture. Only does something if size has changed */
    ScreenCapture::resize(uw, uh);

    /* Start the frame boundary and pass the function to draw */
    static RenderHUD_VDPAU renderHUD;
    renderHUD.setSurface(surface);
    frameBoundary([&] () {orig::VdpPresentationQueueDisplay(presentation_queue, surface, clip_width, clip_height, earliest_presentation_time);}, renderHUD);

    return VDP_STATUS_OK;
}

VdpStatus VdpPresentationQueueBlockUntilSurfaceIdle(VdpPresentationQueue presentation_queue, VdpOutputSurface surface, VdpTime *first_presentation_time)
{
    if (GlobalState::isNative())
        return orig::VdpPresentationQueueBlockUntilSurfaceIdle(presentation_queue, surface, first_presentation_time);

    DEBUGLOGCALL(LCF_WINDOW);

    DeterministicTimer::get().fakeAdvanceTimerFrame();
    // VdpStatus status = orig::VdpPresentationQueueBlockUntilSurfaceIdle(presentation_queue, surface, first_presentation_time);
    // debuglogstdio(LCF_WINDOW, "first_presentation_time %llu", *first_presentation_time);

    return VDP_STATUS_OK;
}

VdpStatus MyVdpGetProcAddress(VdpDevice device, VdpFuncId function_id, void **function_pointer)
{
    DEBUGLOGCALL(LCF_WINDOW);
    VdpStatus status = orig::GetProcAddress(device, function_id, function_pointer);
    switch(function_id) {
        case VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11:
            orig::VdpPresentationQueueTargetCreateX11 = reinterpret_cast<decltype(&VdpPresentationQueueTargetCreateX11)>(*function_pointer);
            *function_pointer = reinterpret_cast<void*>(VdpPresentationQueueTargetCreateX11);
            return status;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE:
            orig::VdpPresentationQueueCreate = reinterpret_cast<decltype(&VdpPresentationQueueCreate)>(*function_pointer);
            *function_pointer = reinterpret_cast<void*>(VdpPresentationQueueCreate);
            return status;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY:
            orig::VdpPresentationQueueDestroy = reinterpret_cast<decltype(&VdpPresentationQueueDestroy)>(*function_pointer);
            *function_pointer = reinterpret_cast<void*>(VdpPresentationQueueDestroy);
            return status;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY:
            orig::VdpPresentationQueueDisplay = reinterpret_cast<decltype(&VdpPresentationQueueDisplay)>(*function_pointer);
            *function_pointer = reinterpret_cast<void*>(VdpPresentationQueueDisplay);
            return status;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE:
            orig::VdpPresentationQueueBlockUntilSurfaceIdle = reinterpret_cast<decltype(&VdpPresentationQueueBlockUntilSurfaceIdle)>(*function_pointer);
            *function_pointer = reinterpret_cast<void*>(VdpPresentationQueueBlockUntilSurfaceIdle);
            return status;
        default:
            break;
    }
    return status;
}

/* Override */ int vdp_device_create_x11(Display *display, int screen, VdpDevice *device, VdpGetProcAddress **get_proc_address)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(vdp_device_create_x11, "vdpau");

    int ret = orig::vdp_device_create_x11(display, screen, device, get_proc_address);

    /* Save the original GetProcAddress function and set our custom function */
    orig::GetProcAddress = *get_proc_address;
    *get_proc_address = MyVdpGetProcAddress;

    /* Gather the functions that will be needed */
    orig::GetProcAddress(*device, VDP_FUNC_ID_BITMAP_SURFACE_CREATE, reinterpret_cast<void**>(&orig::VdpBitmapSurfaceCreate));
    orig::GetProcAddress(*device, VDP_FUNC_ID_BITMAP_SURFACE_DESTROY, reinterpret_cast<void**>(&orig::VdpBitmapSurfaceDestroy));
    orig::GetProcAddress(*device, VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE, reinterpret_cast<void**>(&orig::VdpBitmapSurfacePutBitsNative));
    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE, reinterpret_cast<void**>(&orig::VdpOutputSurfaceRenderBitmapSurface));

    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS, reinterpret_cast<void**>(&orig::VdpOutputSurfaceGetParameters));
    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_CREATE, reinterpret_cast<void**>(&orig::VdpOutputSurfaceCreate));
    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY, reinterpret_cast<void**>(&orig::VdpOutputSurfaceDestroy));
    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE, reinterpret_cast<void**>(&orig::VdpOutputSurfaceRenderOutputSurface));
    orig::GetProcAddress(*device, VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE, reinterpret_cast<void**>(&orig::VdpOutputSurfaceGetBitsNative));

    return ret;
}

}
