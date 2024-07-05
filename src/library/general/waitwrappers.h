/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_WAIT_H_INCL
#define LIBTAS_WAIT_H_INCL

#include "hook.h"

#include <time.h>
#include <unistd.h>
#include <sys/signal.h>
#include <SDL2/SDL.h>
#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <poll.h>

namespace libtas {

/* Poll the file descriptors described by the NFDS structures starting at
   FDS.  If TIMEOUT is nonzero and not -1, allow TIMEOUT milliseconds for
   an event to occur; if TIMEOUT is -1, block until an event occurs.
   Returns the number of file descriptors with events, zero if timed out,
   or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int poll (struct pollfd *fds, nfds_t nfds, int timeout);

/* Like poll, but before waiting the threads signal mask is replaced
   with that specified in the fourth parameter.  For better usability,
   the timeout value is specified using a TIMESPEC object.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int ppoll (struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const __sigset_t *ss);

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

/* Same as above only that the TIMEOUT value is given with higher
   resolution and a sigmask which is been set temporarily.  This version
   should be used.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pselect (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	const struct timespec *timeout, const sigset_t *sigmask);

#ifdef __linux__
/* Wait for events on an epoll instance "epfd". Returns the number of
   triggered events returned in "events" buffer. Or -1 in case of
   error with the "errno" variable set to the specific error code. The
   "events" parameter is a buffer that will contain triggered
   events. The "maxevents" is the maximum number of events to be
   returned ( usually size of "events" ). The "timeout" parameter
   specifies the maximum wait time in milliseconds (-1 == infinite).

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int epoll_wait (int epfd, struct epoll_event *events, int maxevents, int timeout);
#endif
}

#endif
