/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "hook.h"
#include "logging.h"

namespace libtas {

DEFINE_ORIG_POINTER(XDisplayHeight);
DEFINE_ORIG_POINTER(XDisplayWidth);

int XDisplayHeight(Display* display, int screen_number)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (shared_config.screen_height) {
        return shared_config.screen_height;
    }

    LINK_NAMESPACE_GLOBAL(XDisplayHeight);
    return orig::XDisplayHeight(display, screen_number);
}

int XDisplayWidth(Display* display, int screen_number)
{
    DEBUGLOGCALL(LCF_WINDOW);

    if (shared_config.screen_width) {
        return shared_config.screen_width;
    }

    LINK_NAMESPACE_GLOBAL(XDisplayWidth);
    return orig::XDisplayWidth(display, screen_number);
}

}
