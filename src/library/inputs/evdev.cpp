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

#include "evdev.h"
#include "../logging.h"
#include <cstdio>
#include <cerrno>
#include "../DeterministicTimer.h"
#include "../../shared/AllInputs.h"
#include <unistd.h>

namespace libtas {

/* The subarray contains pipe in fd, pipe out fd, and refcount. */
static int evdevfds[AllInputs::MAXJOYS][3] = {{0}};

int is_evdev(const char* source)
{
    /* Extract the ev number from the dev filename */
    int evnum;
    int ret = sscanf(source, "/dev/input/event%d", &evnum);
    if (ret != 1)
        return -1;

    if (evnum < 0 || evnum >= shared_config.nb_controllers) {
        return 0;
    }

    return 1;
}

int open_evdev(const char* source, int flags)
{
    /* Extract the ev number from the dev filename */
    int evnum;
    int ret = sscanf(source, "/dev/input/event%d", &evnum);
    MYASSERT(ret == 1)

    /* Return -1 and set errno if we don't support this ev number */
    if (evnum < 0 || evnum >= shared_config.nb_controllers) {
        errno = ENOENT;
        return -1;
    }

    debuglog(LCF_JOYSTICK, "   evdev device ", evnum, " detected");

    if (evdevfds[evnum][2]++ == 0) {
        /* Register that we use EVDEV for joystick inputs */
        game_info.joystick = GameInfo::EVDEV;
        game_info.tosend = true;

        /* Create an unnamed pipe. */
        MYASSERT(pipe(evdevfds[evnum]) == 0);

        /* Write the synthetic events corresponding to the initial state of the
         * joystick. */
        // struct js_event ev;
        //
        // struct timespec ts = detTimer.getTicks();
        // ev.time = ts.tv_sec*1000 + ts.tv_nsec/1000000;
        // ev.value = 0;
        // for (int button = 0; button < 11; button++) {
        //     ev.type = JS_EVENT_BUTTON | JS_EVENT_INIT;
        //     ev.number = button;
        //     write_jsdev(ev, jsnum);
        // }
        // for (int axis = 0; axis < 8; axis++) {
        //     ev.type = JS_EVENT_AXIS | JS_EVENT_INIT;
        //     ev.number = axis;
        //     write_jsdev(ev, jsnum);
        // }
    }

    return evdevfds[evnum][0];
}

void write_evdev(struct input_event ev, int evnum)
{
    if (evdevfds[evnum][2] != 0)
        write(evdevfds[evnum][1], &ev, sizeof(ev));
}

int get_ev_number(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++)
        if (evdevfds[i][0] == fd)
            return i;
    return -1;
}


int close_evdev(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++) {
        if (evdevfds[i][0] == fd && evdevfds[i][2] != 0 && --evdevfds[i][2] == 0) {
            GlobalNative gn;
            close(evdevfds[i][0]);
            close(evdevfds[i][1]);
            return 0;
        }
    }
    return 1;
}

}
