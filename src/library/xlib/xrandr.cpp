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

#include "xrandr.h"

#include "../hook.h"
#include "../logging.h"
#include "xdisplay.h" // x11::gameDisplays
#include "../global.h"

#include <X11/Xlibint.h> // Xmalloc

namespace libtas {

DEFINE_ORIG_POINTER(XRRGetScreenResources)
DEFINE_ORIG_POINTER(XRRGetScreenResourcesCurrent)
DEFINE_ORIG_POINTER(XRRFreeScreenResources)
DEFINE_ORIG_POINTER(XRRGetOutputInfo)
DEFINE_ORIG_POINTER(XRRFreeOutputInfo)
DEFINE_ORIG_POINTER(XRRGetCrtcInfo)
DEFINE_ORIG_POINTER(XRRFreeCrtcInfo)
DEFINE_ORIG_POINTER(XRRListOutputProperties)
DEFINE_ORIG_POINTER(XRRSetCrtcConfig)
DEFINE_ORIG_POINTER(XRRSetScreenConfig)
DEFINE_ORIG_POINTER(XRRSetScreenConfigAndRate)
DEFINE_ORIG_POINTER(XRRSetScreenSize)

static const char *output_name = "libTAS fake XRR output";
static const char *mode_name = "libTAS fake XRR mode";

XRRScreenResources *XRRGetScreenResources (Display *dpy, Window window)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (Global::shared_config.screen_width) {
        static XRRScreenResources sr;
        sr.ncrtc = 1;
        static RRCrtc sr_crtcs = 1;
        sr.crtcs = &sr_crtcs;

        sr.noutput = 1;
        static RROutput sr_outputs = 1;
        sr.outputs = &sr_outputs;

        sr.nmode = 1;
        static XRRModeInfo sr_mode;
        sr_mode.id = 1;
        sr_mode.width = Global::shared_config.screen_width;
        sr_mode.height = Global::shared_config.screen_height;

        sr_mode.hSyncStart = Global::shared_config.screen_height + 88;
        sr_mode.hSyncEnd = Global::shared_config.screen_height + 132;
        sr_mode.hTotal = Global::shared_config.screen_height + 280;
        sr_mode.vSyncStart = Global::shared_config.screen_width + 4;
        sr_mode.vSyncEnd = Global::shared_config.screen_width + 9;
        sr_mode.vTotal = Global::shared_config.screen_width + 45;
        unsigned int dots = sr_mode.hTotal * sr_mode.vTotal;
        sr_mode.dotClock = dots * Global::shared_config.framerate_num / Global::shared_config.framerate_den - dots / 2;
        sr_mode.name = const_cast<char*>(mode_name);
        sr_mode.nameLength = strlen(mode_name);

        sr.modes = &sr_mode;
        return &sr;
    }

    LINK_NAMESPACE_FULLNAME(XRRGetScreenResources, "libXrandr.so.2");
    XRRScreenResources* sr = orig::XRRGetScreenResources(dpy, window);

    /* Replace the monitor refresh rate */
    for (int m = 0; m < sr->nmode; m++) {
        unsigned int dots = sr->modes[m].hTotal * sr->modes[m].vTotal;
        sr->modes[m].dotClock = dots * Global::shared_config.framerate_num / Global::shared_config.framerate_den - dots / 2;
    }
    return sr;
}

XRRScreenResources *XRRGetScreenResourcesCurrent (Display *dpy, Window window)
{
    DEBUGLOGCALL(LCF_WINDOW);
    return XRRGetScreenResources(dpy, window);
}

void XRRFreeScreenResources (XRRScreenResources *resources)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!Global::shared_config.screen_width) {
        LINK_NAMESPACE_FULLNAME(XRRFreeScreenResources, "libXrandr.so.2");
        return orig::XRRFreeScreenResources(resources);
    }
}

XRROutputInfo *XRRGetOutputInfo (Display *dpy, XRRScreenResources *resources, RROutput output)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (Global::shared_config.screen_width) {
        static XRROutputInfo output_info;
        output_info.crtc = 1;
        output_info.name = const_cast<char*>(output_name);
        output_info.nameLen = strlen(output_name);
        output_info.ncrtc = 1;
        static RRCrtc output_crtcs = 1;
        output_info.crtcs = &output_crtcs;
        output_info.nmode = 1;
        static RRMode output_mode = 1;
        output_info.modes = &output_mode;
        return &output_info;
    }
    LINK_NAMESPACE_FULLNAME(XRRGetOutputInfo, "libXrandr.so.2");
    return orig::XRRGetOutputInfo(dpy, resources, output);
}

void XRRFreeOutputInfo (XRROutputInfo *outputInfo)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!Global::shared_config.screen_width) {
        LINK_NAMESPACE_FULLNAME(XRRFreeOutputInfo, "libXrandr.so.2");
        return orig::XRRFreeOutputInfo(outputInfo);
    }
}

XRRCrtcInfo *XRRGetCrtcInfo (Display *dpy, XRRScreenResources *resources, RRCrtc crtc)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (Global::shared_config.screen_width) {
        static XRRCrtcInfo crtcInfo;
        crtcInfo.x = 0;
        crtcInfo.y = 0;
        crtcInfo.width = Global::shared_config.screen_width;
        crtcInfo.height = Global::shared_config.screen_height;
        crtcInfo.mode = 1;
        crtcInfo.noutput = 1;

        crtcInfo.noutput = 1;
        static RROutput crtc_outputs = 1;
        crtcInfo.outputs = &crtc_outputs;

        crtcInfo.npossible = 1;
        static RROutput crtc_possible = 1;
        crtcInfo.possible = &crtc_possible;

        return &crtcInfo;
    }
    LINK_NAMESPACE_FULLNAME(XRRGetCrtcInfo, "libXrandr.so.2");
    return orig::XRRGetCrtcInfo(dpy, resources, crtc);
}

void XRRFreeCrtcInfo (XRRCrtcInfo *crtcInfo)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!Global::shared_config.screen_width) {
        LINK_NAMESPACE_FULLNAME(XRRFreeCrtcInfo, "libXrandr.so.2");
        return orig::XRRFreeCrtcInfo(crtcInfo);
    }
}

Atom *XRRListOutputProperties (Display *dpy, RROutput output, int *nprop)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!Global::shared_config.screen_width) {
        LINK_NAMESPACE_FULLNAME(XRRListOutputProperties, "libXrandr.so.2");
        return orig::XRRListOutputProperties(dpy, output, nprop);
    }
    
    *nprop = 0;

    /* We need to return something that will be called with XFree() */
    return static_cast<Atom*>(Xmalloc(8));
}

Status XRRSetCrtcConfig (Display *dpy, XRRScreenResources *resources, RRCrtc crtc, Time timestamp, int x, int y, RRMode mode, Rotation rotation, RROutput *outputs, int noutputs)
{
    DEBUGLOGCALL(LCF_WINDOW);
    return RRSetConfigSuccess;
}

Status XRRSetScreenConfig (Display *dpy, XRRScreenConfiguration *config, Drawable draw, int size_index, Rotation rotation, Time timestamp)
{
    DEBUGLOGCALL(LCF_WINDOW);
    return RRSetConfigSuccess;
}

Status XRRSetScreenConfigAndRate (Display *dpy, XRRScreenConfiguration *config, Drawable draw, int size_index, Rotation rotation, short rate, Time timestamp)
{
    DEBUGLOGCALL(LCF_WINDOW);
    return RRSetConfigSuccess;
}

void XRRSetScreenSize (Display *dpy, Window window, int width, int height, int mmWidth, int mmHeight)
{
    DEBUGLOGCALL(LCF_WINDOW);
    /* We prevent games from changing the screen size */
}

void get_monitor_resolution(int& width, int& height)
{
    LINK_NAMESPACE_FULLNAME(XRRGetScreenResourcesCurrent, "libXrandr.so.2");
    LINK_NAMESPACE_FULLNAME(XRRGetCrtcInfo, "libXrandr.so.2");
    LINK_NAMESPACE_FULLNAME(XRRFreeCrtcInfo, "libXrandr.so.2");
    LINK_NAMESPACE_FULLNAME(XRRFreeScreenResources, "libXrandr.so.2");

    XRRScreenResources *screen_resources = nullptr;
    XRRCrtcInfo *crtc_info = nullptr;
    
    for (int d=0; d<GAMEDISPLAYNUM; d++) {
        if (x11::gameDisplays[d]) {
            screen_resources = orig::XRRGetScreenResourcesCurrent(x11::gameDisplays[d], DefaultRootWindow(x11::gameDisplays[d]));
            crtc_info = orig::XRRGetCrtcInfo(x11::gameDisplays[d], screen_resources, screen_resources->crtcs[0]);
            break;
        }
    }
    
    width = crtc_info->width;
    height = crtc_info->height;
    
    if (crtc_info)
        orig::XRRFreeCrtcInfo(crtc_info);
    if (screen_resources)
        orig::XRRFreeScreenResources(screen_resources);
}

}
