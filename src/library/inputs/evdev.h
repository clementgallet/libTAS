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

#ifndef LIBTAS_EVDEV_H_INCL
#define LIBTAS_EVDEV_H_INCL

#include "../global.h"
#include <linux/input.h>

namespace libtas {

/* Is the file a valid jsdev path. Returns -1 for not a jsdev, 0 for a jsdev
 * with invalid number and 1 for valid number
 */
int is_evdev(const char* source);

/* Open a fake dev file using SYS_memfd_create */
int open_evdev(const char* source, int flags);

/* Write an input event in the file */
void write_evdev(struct input_event ev, int jsnum);

/* Get the joystick number from the file descriptor */
int get_ev_number(int fd);

/* Unregister the file descriptor when file is closed */
bool unref_evdev(int fd);

}

#endif
