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

#include "xcbconnection.h"
#include "../hook.h"
#include "../logging.h"
#include "XcbEventQueueList.h"

namespace libtas {

DEFINE_ORIG_POINTER(xcb_connect)
DEFINE_ORIG_POINTER(xcb_disconnect)

xcb_connection_t *xcb_connect(const char *displayname, int *screenp)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(xcb_connect);

    xcb_connection_t* c = orig::xcb_connect(displayname, screenp);

    int i;
    for (i=0; i<GAMECONNECTIONNUM; i++) {
        if (!gameConnections[i]) {
            gameConnections[i] = c;
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
        if (gameConnections[i] == c) {
            gameConnections[i] = nullptr;
            break;
        }
    }

    /* Delete event queue */
    xcbEventQueueList.deleteQueue(c);

    return orig::xcb_disconnect(c);
}

}
