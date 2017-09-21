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

#include "jsdev.h"
#include "../logging.h"
#include <cstdio>
#include <cerrno>
#include "../DeterministicTimer.h"
#include "../../shared/AllInputs.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace libtas {

static int jsdevfds[AllInputs::MAXJOYS] = {0};

int open_jsdev(const char* source, int flags)
{
    /* Extract the js number from the dev filename */
    int jsnum;
    int ret = sscanf(source, "/dev/input/js%d", &jsnum);
    MYASSERT(ret == 1)

    /* Return -1 and set errno if we don't support this js number */
    if (jsnum < 0 || jsnum >= shared_config.nb_controllers) {
        errno = ENOENT;
        return -1;
    }

    if (jsdevfds[jsnum] != 0) {
        debuglog(LCF_JOYSTICK | LCF_ERROR, "Warning, jsdev ", source, " opened multiple times!");
    }

    /* Create an anonymous file and store its file descriptor using the
     * recent memfd_create syscall.
     */
    int fd = syscall(SYS_memfd_create, source, 0);
    jsdevfds[jsnum] = fd;

    /* Write the synthetic events corresponding to the initial state of the
     * joystick. */
    struct js_event ev;

    struct timespec ts = detTimer.getTicks();
    ev.time = ts.tv_sec*1000 + ts.tv_nsec/1000000;
    ev.value = 0;
    for (int button = 0; button < 11; button++) {
        ev.type = JS_EVENT_BUTTON | JS_EVENT_INIT;
        ev.number = button;
        write(fd, &ev, sizeof(ev));
    }
    for (int axis = 0; axis < 11; axis++) {
        ev.type = JS_EVENT_AXIS | JS_EVENT_INIT;
        ev.number = axis;
        write(fd, &ev, sizeof(ev));
    }

    return fd;
}

void write_jsdev(struct js_event ev, int jsnum)
{
    if (jsdevfds[jsnum] != 0) {
        write(jsdevfds[jsnum], &ev, sizeof(ev));
    }
}

void close_jsdev(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++) {
        if (jsdevfds[i] == fd) {
            jsdevfds[i] = 0;
            //return close(fd);
        }
    }

//    debuglog(LCF_JOYSTICK | LCF_ERROR, "Could not find the joyfd to close!");
//    return close(fd);
}

}
