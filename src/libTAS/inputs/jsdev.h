/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_JSDEV_H_INCL
#define LIBTAS_JSDEV_H_INCL

#include "../global.h"
#include <sys/ioctl.h>
#include <linux/joystick.h>

namespace libtas {

int open_jsdev(const char* source, int flags);
void write_jsdev(struct js_event ev, int jsnum);
void close_jsdev(int fd);

}

#endif
