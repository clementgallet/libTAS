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

#include "xdisplay.h"
#include "../hook.h"
#include "../logging.h"
#include "xatom.h"
#include "XlibEventQueueList.h"
#include "../global.h"
#include "../GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(XOpenDisplay)
DEFINE_ORIG_POINTER(XCloseDisplay)
DEFINE_ORIG_POINTER(XDisplayHeight)
DEFINE_ORIG_POINTER(XDisplayWidth)

Display* x11::gameDisplays[GAMEDISPLAYNUM] = {};

Display *XOpenDisplay(const char *display_name)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(XOpenDisplay);

    Display* display;
    OWNCALL(display = orig::XOpenDisplay(display_name));

    if (!display) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "Could not open X connection to %s", display_name ? display_name : "$DISPLAY");
        return nullptr;
    }

    int i;
    for (i=0; i<GAMEDISPLAYNUM; i++) {
        if (!x11::gameDisplays[i]) {
            x11::gameDisplays[i] = display;
            break;
        }
    }
    if (i == GAMEDISPLAYNUM) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "   Reached the limit of registered X connections");
    }

    /* Initialize atoms */
    initX11Atoms(display);

    /* Create event queue */
    xlibEventQueueList.newQueue(display);

    return display;
}

int XCloseDisplay(Display *display)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(XCloseDisplay);

    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i] == display) {
            x11::gameDisplays[i] = nullptr;
            break;
        }
    }

    /* Delete event queue */
    xlibEventQueueList.deleteQueue(display);

    return orig::XCloseDisplay(display);
}

int XDisplayHeight(Display* display, int screen_number)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (Global::shared_config.screen_height) {
        return Global::shared_config.screen_height;
    }

    LINK_NAMESPACE_GLOBAL(XDisplayHeight);
    return orig::XDisplayHeight(display, screen_number);
}

int XDisplayWidth(Display* display, int screen_number)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (Global::shared_config.screen_width) {
        return Global::shared_config.screen_width;
    }

    LINK_NAMESPACE_GLOBAL(XDisplayWidth);
    return orig::XDisplayWidth(display, screen_number);
}

}
