/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xcursor.h"

#include "hook.h"
#include "logging.h"
#include "GlobalState.h"

namespace libtas {

/* Override */ int XDefineCursor(Display* d, Window w, Cursor c)
{
    RETURN_IF_NATIVE(XDefineCursor, (d, w, c), nullptr);
    LOGTRACE_SIMPLE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ int XUndefineCursor(Display* d, Window w)
{
    RETURN_IF_NATIVE(XUndefineCursor, (d, w), nullptr);
    LOGTRACE_SIMPLE(LCF_MOUSE);
    return 0; // Not sure what to return
}

/* Override */ Cursor XcursorLibraryLoadCursor (Display *dpy, const char *file)
{
    LOGTRACE_SIMPLE(LCF_WINDOW | LCF_MOUSE);
    /* Ruffle calls this, but it crashes when called multiple times (due to
     * savestates). Because we already noop XDefineCursor, we can return 0 here */
    return 0;    
}

/* Override */ int XFreeCursor(Display *dpy, Cursor cursor)
{
    LOGTRACE_SIMPLE(LCF_WINDOW | LCF_MOUSE);
    /* When preventing cursor loading, we return an identifier of 0. So we
     * skip calling this function for this particular value. */
    if (cursor == 0)
        return 0;

    RETURN_NATIVE(XFreeCursor, (dpy, cursor), nullptr);
}

}
