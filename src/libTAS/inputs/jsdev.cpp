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

#include "jsdev.h"
#include "../logging.h"
#include <cstdio>
#include <cerrno>
#include "../DeterministicTimer.h"
#include "../../shared/AllInputs.h"
#include "../../shared/SingleInput.h"
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

    /* Register that we use JSDEV for joystick inputs */
    game_info.joystick = GameInfo::JSDEV;
    game_info.tosend = true;

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
        write_jsdev(ev, jsnum);
    }
    for (int axis = 0; axis < 8; axis++) {
        ev.type = JS_EVENT_AXIS | JS_EVENT_INIT;
        ev.number = axis;
        write_jsdev(ev, jsnum);
    }

    return fd;
}

void write_jsdev(struct js_event ev, int jsnum)
{
    int fd = jsdevfds[jsnum];

    if (fd == 0)
        return;

    /* We must simulate a queue using our memfd structure (basically a file).
     * Of course, we must not grow our file too much.
     * One thing that helps us is that we can insert events of the same frame
     * at any order. To acheive that, we insert the event backward from
     * the file position if the next event is from the same frame (or eof).
     *
     *    <---------------------|~~~~~~~~~~~~~~~~~~~|------------------------>
     *            read         new       new       old         unread
     *                       position   event    position
     *
     * For any other case, we append at the end.
     *
     *    <|---------------------------------------------------|+++++++++++++>
     *  old/new                 unread                            new event
     *  position
     *
     */

    off_t curpos = lseek(fd, 0, SEEK_CUR);
    MYASSERT(curpos >= 0)

    off_t jsev_size = static_cast<off_t>(sizeof(struct js_event));

    if (curpos > 0) {
        MYASSERT(curpos >= jsev_size)

        /* Check if we have an event stored after this one */
        off_t lastpos = lseek(fd, 0, SEEK_END);
        MYASSERT(lastpos >= 0)

        if (lastpos == curpos) {
            /* EOF, insert this event backward */
            lseek(fd, lastpos - jsev_size, SEEK_SET);
            write(jsdevfds[jsnum], &ev, jsev_size);
            lseek(fd, lastpos - jsev_size, SEEK_SET);
            debuglog(LCF_JOYSTICK | LCF_EVENTS, "    Wrote jsdev event at offset ", lastpos - jsev_size);
            return;
        }

        /* Read the next event and check if time match */
        MYASSERT((lastpos - curpos) >= jsev_size)
        struct js_event next_ev;
        lseek(fd, curpos, SEEK_SET);
        read(fd, &next_ev, sizeof(next_ev));

        if (next_ev.time == ev.time) {
            /* Events are from the same frame, insert this event backward */
            lseek(fd, curpos - jsev_size, SEEK_SET);
            write(jsdevfds[jsnum], &ev, jsev_size);
            lseek(fd, curpos - jsev_size, SEEK_SET);
            debuglog(LCF_JOYSTICK | LCF_EVENTS, "    Wrote jsdev event at offset ", curpos - jsev_size);
            return;
        }
    }

    /* In any other case, insert the event at the end of the file */
    off_t writepos = lseek(fd, 0, SEEK_END);
    write(jsdevfds[jsnum], &ev, jsev_size);
    debuglog(LCF_JOYSTICK | LCF_EVENTS, "    Wrote jsdev event at offset ", writepos);
    lseek(fd, curpos, SEEK_SET);
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
