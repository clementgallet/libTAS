/*
 * This source file is documented using Doxygen markup.
 * See http://www.stack.nl/~dimitri/doxygen/
 */

/*
 * This copyright notice applies to this header file:
 *
 * Copyright (c) 2008-2009 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file vdpau_x11.h
 * \brief X11 Window System Integration Layer
 *
 * This file contains the \ref api_winsys_x11 X11 Window System
 * Integration Layer.
 */

#ifndef _VDPAU_X11_H
#define _VDPAU_X11_H

#include <X11/Xlib.h>
#include "vdpau.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup api_winsys
 * @{
 */

/**
 * \defgroup api_winsys_x11 X11 Window System Integration Layer
 *
 * The set of VDPAU functionality specific to usage with the X
 * Window System.
 *
 * \section Driver Library Layout
 *
 * An X11-oriented VDPAU installation consists of the following
 * components:
 *
 * - Header files. These files are located in the standard
 *   system header file path.
 *   - \c vdpau/vdpau.h
 *   - \c vdpau/vdpau_x11.h
 * - The VDPAU wrapper library. These files are located in the
 *   standard system (possibly X11-specific) library path.
 *   - \c libvdpau.so.1 (runtime)
 *   - \c libvdpau.so (development)
 * - Back-end driver files. These files are located in a
 *   system-defined library path, which is configurable at compile
 *   time but is typically /usr/lib/vdpau.  Use `pkg-config
 *   --variable=moduledir vdpau` to locate the driver install path.
 *   - \c $moduledir/libvdpau_\%s.so.1
 *   For example:
 *   - \c /usr/lib/vdpau/libvdpau_nvidia.so.1
 *   - \c /usr/lib/vdpau/libvdpau_intel.so.1
 *   - \c /usr/lib/vdpau/libvdpau_ati.so.1
 *   The library path can be overridden by the VDPAU_DRIVER_PATH
 *   environment variable.
 *
 * The VDPAU wrapper library implements just one function; \ref
 * vdp_device_create_x11. The wrapper implements this function by
 * dynamically loading the appropriate back-end driver file mentioned
 * above. When available, the wrapper uses the DRI2 extension's
 * DRI2Connect request with the driver type 'DRI2DriverVDPAU' to
 * determine which back-end driver to load. If that fails, the wrapper
 * library hard-codes the driver name as "nvidia", although this can
 * be overridden using the environment variable VDPAU_DRIVER.
 *
 * The back-end driver is expected to implement a function named
 * \b vdp_imp_device_create_x11. The wrapper will call this function to
 * actually implement the \ref vdp_device_create_x11 application call.
 *
 * Note that it is theoretically possible for an application to
 * create multiple \ref VdpDevice "VdpDevice" objects. In this
 * case, the wrapper library may load multiple back-end drivers
 * into the same application, and/or invoke a specific back-end
 * driver's \b VdpImpDeviceCreateX11 multiple times. The wrapper
 * library imposes no policy regarding whether the application
 * may instantiate multiple \ref VdpDevice "VdpDevice" objects for
 * the same display and/or screen. However, back-end drivers are
 * free to limit the number of \ref VdpDevice "VdpDevice" objects
 * as required by their implementation.
 *
 * @{
 */

/**
 * \brief Create a VdpPresentationQueueTarget for use with X11.
 * \param[in] device The device that will contain the queue
 *       target.
 * \param[in] drawable The X11 Drawable that the presentation
 *       queue will present into.
 * \param[out] target The new queue target's handle.
 * \return VdpStatus The completion status of the operation.
 *
 * Note: VDPAU expects to own the entire drawable for the duration of time
 * that the presentation queue target exists. In particular,
 * implementations may choose to manipulate client-visible X11 window state
 * as required. As such, it is recommended that applications create a
 * dedicated window for the presentation queue target, as a child
 * (grand-child, ...) of their top-level application window.
 *
 * Applications may also create child-windows of the presentation queue
 * target, which will cover any presented video in the normal fashion. VDPAU
 * implementations will not manipulate such child windows in any fashion.
 */
typedef VdpStatus VdpPresentationQueueTargetCreateX11(
    VdpDevice                   device,
    Drawable                    drawable,
    /* output parameters follow */
    VdpPresentationQueueTarget * target
);

/** \hideinitializer */
#define VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11 (VdpFuncId)(VDP_FUNC_ID_BASE_WINSYS + 0)

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif
