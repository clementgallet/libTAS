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

#include "jsdev.h"
#include "../logging.h"
#include <cstdio>
#include <cerrno>
#include <utility>
#include "../DeterministicTimer.h"
#include "../fileio/FileHandleList.h"
#include "../../shared/AllInputs.h"
#include <unistd.h> /* write */
#include "../global.h"
#include "../GlobalState.h"

namespace libtas {

/* The tuple contains pipe in fd, pipe out fd, and then refcount. */
static std::pair<std::pair<int, int>, int> jsdevfds[AllInputs::MAXJOYS];

int is_jsdev(const char* source)
{
    /* Extract the js number from the dev filename */
    int jsnum;
    int ret = sscanf(source, "/dev/input/js%d", &jsnum);
    if (ret != 1)
        return -1;

    if (jsnum < 0 || jsnum >= Global::shared_config.nb_controllers) {
        return 0;
    }

    return 1;
}

int open_jsdev(const char* source, int flags)
{
    /* Extract the js number from the dev filename */
    int jsnum;
    int ret = sscanf(source, "/dev/input/js%d", &jsnum);
    MYASSERT(ret == 1)

    /* Return -1 and set errno if we don't support this js number */
    if (jsnum < 0 || jsnum >= Global::shared_config.nb_controllers) {
        errno = ENOENT;
        return -1;
    }

    debuglogstdio(LCF_JOYSTICK, "   jsdev device %d detected", jsnum);

    if (jsdevfds[jsnum].second++ == 0) {
        /* Register that we use JSDEV for joystick inputs */
        Global::game_info.joystick |= GameInfo::JSDEV;
        Global::game_info.tosend = true;

        /* Create an unnamed pipe */
        jsdevfds[jsnum].first = FileHandleList::createPipe(flags);

        /* If pipe creation failed (e.g. when opening the dev file in write mode),
         * invalidate the pipe and return -1. */
        if (jsdevfds[jsnum].first.first == -1) {
            debuglogstdio(LCF_JOYSTICK, "   could not create jsdev pipe with flags %d", flags);
            jsdevfds[jsnum].second = 0;
            return -1;
        }

        /* Write the synthetic events corresponding to the initial state of the
         * joystick. */
        struct js_event ev;

        struct timespec ts = detTimer.getTicks();
        ev.time = ts.tv_sec*1000 + ts.tv_nsec/1000000;
        ev.value = 0;
        for (int button = 0; button < 11; button++) {
            ev.type = JS_EVENT_BUTTON | JS_EVENT_INIT;
            ev.number = button;
            write_jsdev(ev, jsnum);
        }
        for (int axis = 0; axis < 8; axis++) {
            ev.type = JS_EVENT_AXIS | JS_EVENT_INIT;
            ev.number = axis;
            write_jsdev(ev, jsnum);
        }
    }

    return jsdevfds[jsnum].first.first;
}

void write_jsdev(struct js_event ev, int jsnum)
{
    if (jsdevfds[jsnum].second == 0)
        return;

    /* Check pipe size and don't write if too big */
    int pipeSize;
    NATIVECALL(MYASSERT(ioctl(jsdevfds[jsnum].first.first, FIONREAD, &pipeSize) == 0));

    if (pipeSize < static_cast<int>(64*sizeof(ev)))
        write(jsdevfds[jsnum].first.second, &ev, sizeof(ev));
    else {
        debuglogstdio(LCF_JOYSTICK | LCF_WARNING, "did not write jsdev event, too many already.");
    }
}

bool sync_jsdev(int jsnum)
{
    if (jsdevfds[jsnum].second == 0)
        return false;

    /* Do not attempt to sync if the pipe is already full */
    int attempts = 0, count = 0;
    NATIVECALL(ioctl(jsdevfds[jsnum].first.first, FIONREAD, &count));

    if (count >= static_cast<int>(64*sizeof(struct js_event)))
        return false;

    do {
        NATIVECALL(ioctl(jsdevfds[jsnum].first.first, FIONREAD, &count));
        if (count > 0) {
            if (++attempts > 10 * 100) {
                debuglogstdio(LCF_JOYSTICK | LCF_ERROR | LCF_ALERT, "jsdev sync took too long, were asynchronous events incorrectly enabled?");
                return false;
            }
            struct timespec sleepTime = { 0, 10 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
        }
    } while (count > 0);

    return true;
}

int get_js_number(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++)
        if (jsdevfds[i].second != 0 && jsdevfds[i].first.first == fd)
            return i;
    return -1;
}

bool unref_jsdev(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++)
        if (jsdevfds[i].second != 0 && jsdevfds[i].first.first == fd)
            return --jsdevfds[i].second == 0;
    return true;
}

}
