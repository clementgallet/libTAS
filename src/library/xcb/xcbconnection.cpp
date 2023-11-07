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

#include "xcbconnection.h"
#include "XcbEventQueueList.h"

#include "hook.h"
#include "logging.h"
#include "GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(xcb_connect)
DEFINE_ORIG_POINTER(xcb_disconnect)

xcb_connection_t* x11::gameConnections[GAMECONNECTIONNUM] = {};

xcb_connection_t *xcb_connect(const char *displayname, int *screenp)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(xcb_connect);

    xcb_connection_t* c;
    OWNCALL(c = orig::xcb_connect(displayname, screenp));

    if (!c) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "Could not open xcb connection to %s (%d)", displayname ? displayname : "$DISPLAY", screenp ? *screenp : 0);
        return nullptr;
    }

    int i;
    for (i=0; i<GAMECONNECTIONNUM; i++) {
        if (!x11::gameConnections[i]) {
            x11::gameConnections[i] = c;
            break;
        }
    }
    if (i == GAMECONNECTIONNUM) {
        debuglogstdio(LCF_WINDOW | LCF_ERROR, "   Reached the limit of registered xcb connections");
    }

    /* Initialize atoms */
    // initX11Atoms(display);

    /* Create event queue */
    xcbEventQueueList.newQueue(c);

    return c;
}

void xcb_disconnect(xcb_connection_t *c)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(xcb_disconnect);

    for (int i=0; i<GAMECONNECTIONNUM; i++) {
        if (x11::gameConnections[i] == c) {
            x11::gameConnections[i] = nullptr;
            break;
        }
    }

    /* Delete event queue */
    xcbEventQueueList.deleteQueue(c);

    return orig::xcb_disconnect(c);
}

}
