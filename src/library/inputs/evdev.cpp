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

#include "evdev.h"
#include "../logging.h"
#include <cstdio>
#include <cerrno>
#include <utility>
#include "../DeterministicTimer.h"
#include "../fileio/FileHandleList.h"
#include "../../shared/AllInputs.h"
#include <unistd.h> /* write */

namespace libtas {

/* The tuple contains pipe in fd, pipe out fd, and then refcount. */
static std::pair<std::pair<int, int>, int> evdevfds[AllInputs::MAXJOYS];

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

    if (evdevfds[evnum].second++ == 0) {
        /* Register that we use EVDEV for joystick inputs */
        game_info.joystick |= GameInfo::EVDEV;
        game_info.tosend = true;

        /* Create an unnamed pipe. */
        evdevfds[evnum].first = FileHandleList::createPipe();
    }

    return evdevfds[evnum].first.first;
}

void write_evdev(struct input_event ev, int evnum)
{
    if (evdevfds[evnum].second == 0)
        return;

    /* Check pipe size and don't write if too big */
    int pipeSize;
    NATIVECALL(MYASSERT(ioctl(evdevfds[evnum].first.first, FIONREAD, &pipeSize) == 0));

    if (pipeSize < (64*sizeof(ev)))
        write(evdevfds[evnum].first.second, &ev, sizeof(ev));
    else {
        debuglog(LCF_JOYSTICK | LCF_WARNING, "did not write evdev event, too many already.");
    }
}

bool sync_evdev(int evnum)
{
    if (evdevfds[evnum].second == 0)
        return false;

    int attempts = 0, count = 0;
    NATIVECALL(ioctl(evdevfds[evnum].first.first, FIONREAD, &count));

    if (count >= (64*sizeof(struct input_event)))
        return false;

    do {
        NATIVECALL(ioctl(evdevfds[evnum].first.first, FIONREAD, &count));
        if (count > 0) {
            if (++attempts > 100 * 100) {
                debuglog(LCF_JOYSTICK | LCF_ERROR | LCF_ALERT, "evdev sync took too long, were asynchronous events incorrectly enabled?");
                return false;
            }
            struct timespec sleepTime = { 0, 10 * 1000 };
            NATIVECALL(nanosleep(&sleepTime, NULL));
        }
    } while (count > 0);

    return true;
}

int get_ev_number(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++)
        if (evdevfds[i].second != 0 && evdevfds[i].first.first == fd)
            return i;
    return -1;
}

bool unref_evdev(int fd)
{
    for (int i=0; i<AllInputs::MAXJOYS; i++)
        if (evdevfds[i].second != 0 && evdevfds[i].first.first == fd)
            return --evdevfds[i].second == 0;
    return true;
}

}
