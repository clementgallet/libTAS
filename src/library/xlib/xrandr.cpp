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

#include "xrandr.h"

#include "../hook.h"
#include "../logging.h"

namespace libtas {

DEFINE_ORIG_POINTER(XRRGetScreenResources);
DEFINE_ORIG_POINTER(XRRGetScreenResourcesCurrent);
DEFINE_ORIG_POINTER(XRRFreeScreenResources);
DEFINE_ORIG_POINTER(XRRGetOutputInfo);
DEFINE_ORIG_POINTER(XRRFreeOutputInfo);
DEFINE_ORIG_POINTER(XRRGetCrtcInfo);
DEFINE_ORIG_POINTER(XRRFreeCrtcInfo);
DEFINE_ORIG_POINTER(XRRListOutputProperties);
DEFINE_ORIG_POINTER(XRRSetCrtcConfig);
DEFINE_ORIG_POINTER(XRRSetScreenConfig);
DEFINE_ORIG_POINTER(XRRSetScreenConfigAndRate);
DEFINE_ORIG_POINTER(XRRSetScreenSize);

static const char *output_name = "libTAS fake XRR output";
static const char *mode_name = "libTAS fake XRR mode";

XRRScreenResources *XRRGetScreenResources (Display *dpy, Window window)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (shared_config.screen_width) {
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
        sr_mode.width = shared_config.screen_width;
        sr_mode.height = shared_config.screen_height;

        sr_mode.hSyncStart = shared_config.screen_height + 88;
        sr_mode.hSyncEnd = shared_config.screen_height + 132;
        sr_mode.hTotal = shared_config.screen_height + 280;
        sr_mode.vSyncStart = shared_config.screen_width + 4;
        sr_mode.vSyncEnd = shared_config.screen_width + 9;
        sr_mode.vTotal = shared_config.screen_width + 45;
        unsigned int dots = sr_mode.hTotal * sr_mode.vTotal;
        sr_mode.dotClock = dots * shared_config.framerate_num / shared_config.framerate_den - dots / 2;
        sr_mode.name = const_cast<char*>(mode_name);
        sr_mode.nameLength = strlen(mode_name);

        sr.modes = &sr_mode;
        return &sr;
    }

    LINK_NAMESPACE(XRRGetScreenResources, "Xrandr");
    XRRScreenResources* sr = orig::XRRGetScreenResources(dpy, window);

    /* Replace the monitor refresh rate */
    for (int m = 0; m < sr->nmode; m++) {
        unsigned int dots = sr->modes[m].hTotal * sr->modes[m].vTotal;
        sr->modes[m].dotClock = dots * shared_config.framerate_num / shared_config.framerate_den - dots / 2;
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
    if (!shared_config.screen_width) {
        LINK_NAMESPACE(XRRFreeScreenResources, "Xrandr");
        return orig::XRRFreeScreenResources(resources);
    }
}

XRROutputInfo *XRRGetOutputInfo (Display *dpy, XRRScreenResources *resources, RROutput output)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (shared_config.screen_width) {
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
    LINK_NAMESPACE(XRRGetOutputInfo, "Xrandr");
    return orig::XRRGetOutputInfo(dpy, resources, output);
}

void XRRFreeOutputInfo (XRROutputInfo *outputInfo)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!shared_config.screen_width) {
        LINK_NAMESPACE(XRRFreeOutputInfo, "Xrandr");
        return orig::XRRFreeOutputInfo(outputInfo);
    }
}

XRRCrtcInfo *XRRGetCrtcInfo (Display *dpy, XRRScreenResources *resources, RRCrtc crtc)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (shared_config.screen_width) {
        static XRRCrtcInfo crtcInfo;
        crtcInfo.x = 0;
        crtcInfo.y = 0;
        crtcInfo.width = shared_config.screen_width;
        crtcInfo.height = shared_config.screen_height;
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
    LINK_NAMESPACE(XRRGetCrtcInfo, "Xrandr");
    return orig::XRRGetCrtcInfo(dpy, resources, crtc);
}

void XRRFreeCrtcInfo (XRRCrtcInfo *crtcInfo)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!shared_config.screen_width) {
        LINK_NAMESPACE(XRRFreeCrtcInfo, "Xrandr");
        return orig::XRRFreeCrtcInfo(crtcInfo);
    }
}

Atom *XRRListOutputProperties (Display *dpy, RROutput output, int *nprop)
{
    DEBUGLOGCALL(LCF_WINDOW);
    if (!shared_config.screen_width) {
        LINK_NAMESPACE(XRRListOutputProperties, "Xrandr");
        return orig::XRRListOutputProperties(dpy, output, nprop);
    }
    
    Atom *ret = orig::XRRListOutputProperties(dpy, output, nprop);
    *nprop = 0;

    /* We need to return something that will be called with XFree() */
    return ret;
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
    LINK_NAMESPACE(XRRGetScreenResourcesCurrent, "Xrandr");
    LINK_NAMESPACE(XRRGetCrtcInfo, "Xrandr");
    LINK_NAMESPACE(XRRFreeCrtcInfo, "Xrandr");
    LINK_NAMESPACE(XRRFreeScreenResources, "Xrandr");

    XRRScreenResources *screen_resources = nullptr;
    XRRCrtcInfo *crtc_info = nullptr;
    
    for (int d=0; d<GAMEDISPLAYNUM; d++) {
        if (gameDisplays[d]) {
            screen_resources = orig::XRRGetScreenResourcesCurrent(gameDisplays[d], DefaultRootWindow(gameDisplays[d]));
            crtc_info = orig::XRRGetCrtcInfo(gameDisplays[d], screen_resources, screen_resources->crtcs[0]);
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
