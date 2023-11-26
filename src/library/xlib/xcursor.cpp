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

#include "xcursor.h"

#include "hook.h"
#include "logging.h"

namespace libtas {

OVERRIDE Cursor XcursorLibraryLoadCursor (Display *dpy, const char *file)
{
    DEBUGLOGCALL(LCF_WINDOW | LCF_MOUSE);
    /* Ruffle calls this, but it crashes when called multiple times (due to
     * savestates). Because we already noop XDefineCursor, we can return 0 here */
    return 0;    
}

}
