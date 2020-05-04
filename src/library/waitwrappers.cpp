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

#include "waitwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "GlobalState.h"
#include "hook.h"
#include "audio/alsa/pcm.h"

namespace libtas {

DEFINE_ORIG_POINTER(poll);
DEFINE_ORIG_POINTER(select);
DEFINE_ORIG_POINTER(pselect);
DEFINE_ORIG_POINTER(epoll_wait);

/* Override */ int poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
    LINK_NAMESPACE_GLOBAL(poll);

    if (GlobalState::isNative()) {
        return orig::poll(fds, nfds, timeout);
    }

    debuglog(LCF_WAIT, __func__, " call with ", nfds, " fds and timeout ", timeout);

    /* Check for the fd used by ALSA */
    for (int i = 0; i < nfds; i++) {
        if (fds[i].fd == 0xa15a) {
            /* Wait here until our ALSA buffer has enough bytes available */
            int ret = snd_pcm_wait(reinterpret_cast<snd_pcm_t*>(static_cast<intptr_t>(fds[i].revents)), timeout);

            if (ret == 1) return ret;

            /* Call the poll on the remaining fds if any */
            if (nfds > 1) {
                fds[i] = fds[nfds-1];
                ret = orig::poll(fds, nfds-1, timeout);

                /* If call returned an event, don't handle the ALSA fd */
                if (ret > 0)
                    return ret;
            }

            return ret;
        }
    }

    return orig::poll(fds, nfds, timeout);
}

/* Override */ int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    LINK_NAMESPACE_GLOBAL(select);

    /* select can be used to sleep the cpu if feed with all null parameters
     * except for timeout. In this case we replace it with what we did for
     * other sleep functions. Otherwise, we call the original function.
     */
    if ((nfds != 0) || (readfds != nullptr) || (writefds != nullptr) || (exceptfds != nullptr))
    {
        return orig::select(nfds, readfds, writefds, exceptfds, timeout);
    }

    if (GlobalState::isNative()) {
        return orig::select(nfds, readfds, writefds, exceptfds, timeout);
    }

    bool mainT = ThreadManager::isMainThread();
    debuglog(LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), __func__, " call - sleep for ", timeout->tv_sec * 1000000 + timeout->tv_usec, " usec");

    /* If the function was called from the main thread, transfer the wait to
     * the timer and do not actually wait.
     */
    if (mainT && (timeout->tv_sec || timeout->tv_usec)) {
        struct timespec ts;
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_usec * 1000;
        detTimer.addDelay(ts);

        NATIVECALL(sched_yield());
        return 0;
    }

    return orig::select(nfds, readfds, writefds, exceptfds, timeout);
}

/* Override */ int pselect (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	const struct timespec *timeout, const __sigset_t *sigmask)
{
    LINK_NAMESPACE_GLOBAL(pselect);

    /* select can be used to sleep the cpu if feed with all null parameters
     * except for timeout. In this case we replace it with what we did for
     * other sleep functions. Otherwise, we call the original function.
     */
    if ((nfds != 0) || (readfds != nullptr) || (writefds != nullptr) || (exceptfds != nullptr))
    {
        return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    }

    if (GlobalState::isNative()) {
        return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    }

    bool mainT = ThreadManager::isMainThread();
    debuglog(LCF_SLEEP | (mainT?LCF_NONE:LCF_FREQUENT), __func__, " call - sleep for ", timeout->tv_sec * 1000000000 + timeout->tv_nsec, " nsec");

    /* If the function was called from the main thread, transfer the wait to
     * the timer and do not actually wait.
     */
    if (mainT && (timeout->tv_sec || timeout->tv_nsec)) {
        detTimer.addDelay(*timeout);

        NATIVECALL(sched_yield());
        return 0;
    }

    return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
}

/* Override */ int epoll_wait (int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    LINK_NAMESPACE_GLOBAL(epoll_wait);

    if (GlobalState::isNative()) {
        return orig::epoll_wait(epfd, events, maxevents, timeout);
    }

    debuglog(LCF_SLEEP, __func__, " call with timeout ", timeout, " msec");

    int ret = orig::epoll_wait(epfd, events, maxevents, timeout);

    /* If the function was called from the main thread and the function timed out,
     * advance the timer. */
    if ((timeout != -1) && (ret == 0) && ThreadManager::isMainThread()) {
        struct timespec ts;
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = 1000000 * (timeout % 1000);
        detTimer.addDelay(ts);
    }

    return ret;
}

}
